#include "SlidingWindowReceiver.hpp"
#include <iostream>
#include <vector>

#define PAYLOAD_SZ 5120

SlidingWindowReceiver::SlidingWindowReceiver(int maxSeqBits, int windowSize_)
    : maxSeqNum(1 << maxSeqBits),
      windowSize(windowSize_),
      baseSeqNum(0),
      transferComplete(false) {
    seqStatusVector.resize(windowSize, {false, ""});
}

void SlidingWindowReceiver::initializeWindow() {
    baseSeqNum = 0;
    transferComplete = false;
    for (auto &entry : seqStatusVector) {
        entry = {false, ""};
    }
}

bool SlidingWindowReceiver::processPacket(int seqNum, const std::string &payload) {
    // Validate if the sequence number is within the current window
    int endSeqNum = (baseSeqNum + windowSize - 1) % maxSeqNum;
    if ((baseSeqNum <= endSeqNum && (seqNum < baseSeqNum || seqNum > endSeqNum)) ||
        (baseSeqNum > endSeqNum && (seqNum < baseSeqNum && seqNum > endSeqNum))) {
        std::cout << "Received out-of-window sequence number: " << seqNum << std::endl;
        return false;
    }

    // Compute the window index for the sequence number
    int index = (seqNum - baseSeqNum + maxSeqNum) % maxSeqNum;
    if (seqStatusVector[index].received) {
        std::cout << "Duplicate packet received: " << seqNum << std::endl;
        return false; // Duplicate packet
    }

    // Store the payload and mark the packet as received
    seqStatusVector[index] = {true, payload};
    std::cout << "Packet " << seqNum << " received and stored." << std::endl;

    return true;
}

void SlidingWindowReceiver::sendAck(int sock, const struct sockaddr *senderAddr, socklen_t addrLen) {
    int ackNum = (baseSeqNum + getNumOfReadyPackets() - 1) % maxSeqNum;
    std::cout << "Sending ACK for sequence number: " << ackNum << std::endl; // Debug log

    if (sendto(sock, &ackNum, sizeof(ackNum), 0, senderAddr, addrLen) < 0) {
        std::cerr << "Failed to send ACK for sequence number: " << ackNum << std::endl;
    }
}



bool SlidingWindowReceiver::hasReadyData() const {
    return seqStatusVector[0].received; // First packet in the window is ready
}

std::string SlidingWindowReceiver::getNextChunk(unsigned int length) {

    
    if (!hasReadyData()) {
        return "";
    }

    // Extract the payload of the first packet in the window
    std::string data = seqStatusVector[0].payload;

    // Slide the window
    seqStatusVector.erase(seqStatusVector.begin());
    seqStatusVector.push_back({false, ""});
    baseSeqNum = (baseSeqNum + 1) % maxSeqNum;

    // Check if the transfer is complete (window empty and no unacknowledged packets)
    if (length < PAYLOAD_SZ) {
    std::cout << "End-of-transfer packet received." << std::endl;
    transferComplete = true;
}

    return data;
}

int SlidingWindowReceiver::getNumOfReadyPackets() const {
    int count = 0;
    for (const auto &entry : seqStatusVector) {
        if (entry.received) {
            ++count;
        } else {
            break; // Stop at the first unreceived packet
        }
    }
    return count;
}


bool SlidingWindowReceiver::isTransferComplete() const {
    return transferComplete;
}

