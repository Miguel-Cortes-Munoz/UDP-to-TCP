#ifndef SLIDINGWINDOWSENDER_H
#define SLIDINGWINDOWSENDER_H

#include <map>
#include <chrono>
#include <string>
#include "ExerciseHeader.h"

class SlidingWindowSender {
private:
    static constexpr int WINDOW_SIZE = 31; //windowsize
    int windowStart;  
    int windowEnd;    
    std::map<int, std::string> sentPackets; //sequence num -> packet data
    std::map<int, std::chrono::time_point<std::chrono::steady_clock>> sentTimes; // sequence num -> sentTimes

public:
    //SlidingWindowSender();

    
    void initializeWindow();
    int getWindowStart() const;

    // add packet to queue
    void addPacket(int seqNum, const std::string& data);

    //send packet in queue
    void transmit(int sock, struct addrinfo* ptr);

    //receive ack and slide the window
    void receiveAck(int ackNum);

    //retransmit the expired packet
    void handleTimeouts(int sock, struct addrinfo* ptr);

    int getWindowEnd();
    int getWindowSize();

};

#endif
