#ifndef SLIDING_WINDOW_RECEIVER_HPP
#define SLIDING_WINDOW_RECEIVER_HPP

#include <string>
#include <vector>
#include <netinet/in.h>  // For sockaddr
#include "ExerciseHeader.h"


class SlidingWindowReceiver {
public:
    // Constructor
    SlidingWindowReceiver(int maxSeqBits, int windowSize_);

    // Initialize the receiver window
    void initializeWindow();

    // Process a received packet
    bool processPacket(int seqNum, const std::string &payload);

    // Send acknowledgment for the highest sequence number in the window
    void sendAck(int sock, const struct sockaddr *senderAddr, socklen_t addrLen);

    // Check if there is any ready data in the window
    bool hasReadyData() const;

    // Get the next chunk of data from the window
    std::string getNextChunk(unsigned int length);

    // Get the number of ready packets in the window
    int getNumOfReadyPackets() const;

    // Check if the window is empty (no received packets)

    // Check if the file transfer is complete
    bool isTransferComplete() const;

private:
    

    // Maximum sequence number (based on sequence bits)
    int maxSeqNum;

    // Size of the sliding window
    int windowSize;

    // Base sequence number of the current window
    int baseSeqNum;

    // Flag indicating if the transfer is complete
    bool transferComplete = false;;

    // A vector of sequence status entries for each packet in the window
    struct SeqEntry {
        bool received;   // Whether the packet is received
        std::string payload;  // The payload of the received packet
    };

    std::vector<SeqEntry> seqStatusVector;  // Window of sequence entries
};

#endif // SLIDING_WINDOW_RECEIVER_HPP
