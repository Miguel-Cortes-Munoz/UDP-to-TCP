#include "ExerciseHeader.h"
#include "SlidingWindowSender.hpp"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <filesystem>
#include <zlib.h>

#define PAYLOAD_SZ 5120

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " destination_host destination_port file_path" << std::endl;
        return 1;
    }
    std::string fileName = argv[3];
    std::filesystem::path filePath = std::filesystem::current_path() / fileName;

    int sock, rval;
    struct addrinfo hints, *results, *ptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;

    if ((rval = getaddrinfo(argv[1], argv[2], &hints, &results)) != 0) {
        std::cerr << "Error getting the destination address: " << gai_strerror(rval) << std::endl;
        return 2;
    }

    for (ptr = results; ptr != NULL; ptr = ptr->ai_next) {
        if ((sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) != -1) {
            break;
        }
    }

    if (ptr == NULL) {
        std::cerr << "Failed to open socket" << std::endl;
        return 3;
    }

    // open file
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file" << std::endl;
        close(sock);
        return 4;
    }
    //initialize window
    SlidingWindowSender sender;
    sender.initializeWindow();

    char buffer[HEADER_SZ + PAYLOAD_SZ] = {0};
    ExerciseHeader header;
    header.setBuffer(buffer, sizeof(buffer));

    unsigned int sequenceNumber = 0;
    while (file) {
        file.read(buffer, PAYLOAD_SZ);
        std::streamsize bytesRead = file.gcount();

        if (bytesRead > 0) {
            sender.addPacket(sequenceNumber++, std::string(buffer, bytesRead));
        }
    }

    file.close();

    //send packet
    sender.transmit(sock, ptr);
    
 
while (sequenceNumber > sender.getWindowStart()) {
    int ackNum = -1;
    struct sockaddr_storage fromAddr;
    socklen_t addrLen = sizeof(fromAddr);
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 1;  // Wait for 1 second
    timeout.tv_usec = 0;

    int ready = select(sock + 1, &readfds, NULL, NULL, &timeout);
    if (ready > 0 && FD_ISSET(sock, &readfds)) {
        int bytesReceived = recvfrom(sock, &ackNum, sizeof(ackNum), 0, (struct sockaddr*)&fromAddr, &addrLen);

        if (bytesReceived > 0) {
            sender.receiveAck(ackNum);  // Process the acknowledgment

            // Add new packets into the window if the window has shifted
            int newSeqNum = sender.getWindowStart() + sender.getWindowSize() - 1;
            while (newSeqNum < sequenceNumber) {
                // Read and add packets for the new sequence numbers
                std::ifstream file(filePath, std::ios::binary);
                file.seekg(newSeqNum * PAYLOAD_SZ, std::ios::beg);  // Skip to the correct position in the file
                char buffer[PAYLOAD_SZ];
                file.read(buffer, PAYLOAD_SZ);
                std::streamsize bytesRead = file.gcount();
                
                if (bytesRead > 0) {
                    sender.addPacket(newSeqNum, std::string(buffer, bytesRead));
                }
                newSeqNum++;
            }

            // After adding packets, trigger the send process again
            sender.transmit(sock, ptr);
        } else {
            std::cerr << "Error in recvfrom: " << strerror(errno) << std::endl;
        }
    } else if (ready == 0) {
        std::cout << "Timeout waiting for ACK, checking timeouts..." << std::endl;
        sender.handleTimeouts(sock, ptr);  // Handle timeouts and retransmissions
    } else {
        std::cerr << "Error in select: " << strerror(errno) << std::endl;
        sender.handleTimeouts(sock, ptr);  // Handle timeouts and retransmissions
    }

    // Handle retransmissions for timed-out packets
    sender.handleTimeouts(sock, ptr);
}



    close(sock);
    freeaddrinfo(results);
    std::cout << "File transfer completed." << std::endl;
    return 0;
}

