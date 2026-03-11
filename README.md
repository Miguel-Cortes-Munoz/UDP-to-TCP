# Reliable File Transfer Protocol (IPv4/IPv6)

This project is a custom file transfer protocol developed in C++ for a networking class. It is capable of transferring files over a local network using either IPv4 or IPv6, based on user selection. The protocol includes reliability mechanisms such as CRC checks, a sliding window, and acknowledgement-based communication.

Developed by Miguel Cortes Muñoz in collaboration with Shunsuke Kobayashi. Worked on it throughout the semester for our Computer Networking course.

## Features

- Supports both IPv4 and IPv6
- Sliding window protocol for efficient and reliable transmission
- CRC32 checks for data integrity
- Acknowledgement-based (ACK) transfer and retransmission
- Operates over local UDP sockets

## Usage

### Receiver

./[program_name] port_number output_file [-4 | -6]

markdown
Copy
Edit

- `port_number`: The port to listen on
- `output_file`: File to save the received content
- `-4` or `-6`: Use IPv4 or IPv6

**Example:**

./receiver 8080 received_file.txt -4

shell
Copy
Edit

### Sender

./[program_name] destination_host destination_port file_path

markdown
Copy
Edit

- `destination_host`: The receiver’s IP address (IPv4 or IPv6)
- `destination_port`: The port the receiver is listening on
- `file_path`: Path to the file to be sent

**Example:**

./sender 127.0.0.1 8080 file_to_send.txt

## How It Works

- The sender splits the file into packets, each with a sequence number and CRC32 checksum.
- The receiver validates each packet using CRC and sends an ACK for valid packets.
- A sliding window allows multiple packets to be in transit and manages retransmissions for any lost or corrupted packets.
- The connection is unidirectional and uses UDP sockets for data transfer.

## Notes

- Intended for use on local networks; not designed for high-latency or high-loss environments.
- Supports IPv4 loopback (`127.0.0.1`) and IPv6 loopback (`::1`)
- No external libraries needed; written using standard C++ and POSIX sockets.

## Authors

- Miguel Cortes Muñoz  
- Shunsuke Kobayashi

Project completed for the Computer Networking course at the University of Lethbridge.
