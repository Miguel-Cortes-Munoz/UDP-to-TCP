#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <filesystem>
#include <zlib.h>
#include "SlidingWindowSender.hpp"
#include "ExerciseHeader.h"

#define PAYLOAD_SZ 5120

void SlidingWindowSender::initializeWindow() {
    windowStart = 0;
    windowEnd = windowStart + WINDOW_SIZE - 1;  //31
    std::cout << "Initialized window: start=" << windowStart
              << ", end=" << windowEnd << " (size=31)" << std::endl;
}

void SlidingWindowSender::addPacket(int seqNum, const std::string& data) {
    if (seqNum >= windowStart && seqNum <= windowEnd) {
        sentPackets[seqNum] = data;  //add packet data
        std::cout << "Added packet " << seqNum << " to queue." << std::endl;
    } else {
        //std::cerr << "Error: Packet " << seqNum << " is outside the current window range." << std::endl;
    }
}

void SlidingWindowSender::transmit(int sock, struct addrinfo* ptr) {
    for (const auto& [seqNum, data] : sentPackets) {
        char buffer[HEADER_SZ + PAYLOAD_SZ] = {0};
        ExerciseHeader header;
        header.setBuffer(buffer, sizeof(buffer));

        //header
        header.setType(1);  //data packet
        header.setSequenceNum(seqNum);
        header.setLength(data.size());

        // CRC calculation
        uLong crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, reinterpret_cast<const Bytef*>(data.data()), data.size());
        header.setCRC(static_cast<uint32_t>(crc));

        // copy payload
        memcpy(buffer + HEADER_SZ, data.data(), data.size());

        //send packet
        sendto(sock, buffer, HEADER_SZ + data.size(), 0, ptr->ai_addr, ptr->ai_addrlen);

        // time stamp
        sentTimes[seqNum] = std::chrono::steady_clock::now();
        std::cout << "Sent packet " << seqNum << std::endl;
    }
}

void SlidingWindowSender::receiveAck(int ackNum) {
    if (ackNum >= windowStart && ackNum <= windowEnd) {
        std::cout << "Received ACK for packet " << ackNum << std::endl;

        for (int seq = windowStart; seq <= ackNum; ++seq) {
    sentPackets.erase(seq);
    sentTimes.erase(seq);
    }
    windowStart = ackNum + 1;
        windowEnd = windowStart + WINDOW_SIZE - 1;

        
    } else {
        std::cerr << "Warning: ACK " << ackNum << " is outside the current window range." << std::endl;
    }
}


int SlidingWindowSender::getWindowStart() const {
    return windowStart;
}

void SlidingWindowSender::handleTimeouts(int sock, struct addrinfo* ptr) {
    
    auto now = std::chrono::steady_clock::now();
               

    for (const auto& [seqNum, sendTime] : sentTimes) {
        
        // time out 1 sec
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - sendTime).count() > 1000) {
            std::cerr << "Timeout for packet " << seqNum << ". Resending..." << std::endl;
            addPacket(seqNum, sentPackets[seqNum]);  // add to send queue
            transmit(sock, ptr);  // re-send
        }
    }
}


int SlidingWindowSender::getWindowEnd(){
    return windowEnd;
}

int SlidingWindowSender::getWindowSize(){
    return (windowStart -  windowEnd);
}