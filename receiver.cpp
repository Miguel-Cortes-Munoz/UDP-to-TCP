#include "ExerciseHeader.h"
#include "SlidingWindowReceiver.hpp"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <zlib.h>
#include <chrono> // For timers

#define PAYLOAD_SZ 5120
#define HEADER_SZ 16
#define ACK_TIMEOUT_MS 100  // Timeout in milliseconds
#define MAX_PACKETS_BEFORE_ACK 30 // Number of packets before sending ACK

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " port_number output_file [-4|-6]" << std::endl;
        return 1;
    }

    int sock, rval;
    struct addrinfo hints, *results;
    memset(&hints, 0, sizeof(hints));
    if (strcmp(argv[3], "-4") == 0) {
        hints.ai_family = AF_INET;
    } else {
        hints.ai_family = AF_INET6;
    }
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rval = getaddrinfo(NULL, argv[1], &hints, &results)) != 0) {
        std::cerr << "Error getting local address: " << gai_strerror(rval) << std::endl;
        return 2;
    }

    sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if (sock == -1) {
        std::cerr << "Failed to open socket" << std::endl;
        return 3;
    }

    if (bind(sock, results->ai_addr, results->ai_addrlen) == -1) {
        std::cerr << "Binding failed" << std::endl;
        close(sock);
        return 4;
    }

    freeaddrinfo(results);

    std::ofstream file(argv[2], std::ios::binary);
    if (!file) {
        std::cerr << "Cannot create output file" << std::endl;
        close(sock);
        return 5;
    }

    char buffer[HEADER_SZ + PAYLOAD_SZ] = {0};
    ExerciseHeader header;
    header.setBuffer(buffer, sizeof(buffer));

    SlidingWindowReceiver receiver(8, 16); // Example: 8-bit sequence numbers, window size of 16
    receiver.initializeWindow();
    unsigned sequenceNumber = header.getSequenceNum();

    
int packetCounter = 0;
unsigned maxSequenceNum = 255;
unsigned increment = 255;

// Timer initialization
auto lastAckTime = std::chrono::high_resolution_clock::now();

while (true) {
    socklen_t fromlen;
    struct sockaddr_storage fromaddr;

    fromlen = sizeof(fromaddr);
    int byteCount = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)(&fromaddr), &fromlen);
    if (byteCount == -1) {
        std::cerr << "Failed to receive packet" << std::endl;
        continue;
    }

    // Extract header information
    unsigned sequenceNumber = header.getSequenceNum();

    unsigned int type = header.getType();
    unsigned int length = header.getLength();

    uint32_t receivedCRC = header.getCRC();

    // Validate packet type
    if (type != 1) {
        std::cerr << "Unexpected packet type: " << type << std::endl;
        continue; // Ignore if not data packet
    }

    // Validate CRC
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, reinterpret_cast<const Bytef *>(buffer + HEADER_SZ), length);
    if (receivedCRC != static_cast<uint32_t>(crc)) {
        std::cerr << "CRC mismatch, packet corrupted" << std::endl;
        continue;
    }

    // Calculate the new sequence number based on packetCounter and increment
    sequenceNumber = (packetCounter * increment) % (maxSequenceNum + 1);

    // Process packet using sliding window
    std::string payload(buffer + HEADER_SZ, length);
    if (receiver.processPacket(sequenceNumber, payload)) {
        packetCounter++;
    }

    // Check if we can write data to file
    while (receiver.hasReadyData()) {
        std::string chunk = receiver.getNextChunk(header.getLength());
        file.write(chunk.data(), chunk.size());
    }

    // Timer check for ACK
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAckTime);

    // Send ACK if conditions are met
    if (packetCounter >= MAX_PACKETS_BEFORE_ACK || elapsedTime.count() >= ACK_TIMEOUT_MS) {
        receiver.sendAck(sock, (struct sockaddr *)&fromaddr, fromlen);

        // Reset timer and packet counter
        lastAckTime = std::chrono::high_resolution_clock::now();
        packetCounter = 0;
    }

    // Exit loop if transfer is complete
    if (receiver.isTransferComplete()) {
        receiver.sendAck(sock, (struct sockaddr *)&fromaddr, fromlen);   
        break;
    }
}

file.close();
close(sock);
return 0;
}