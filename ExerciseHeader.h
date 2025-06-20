#ifndef __EXERCISEHEADER_H
#define __EXERCISEHEADER_H

#include <cstdint>
// maximum size for the payload 
#define DATA_SZ 1024
// size of header
#define HEADER_SZ 16


struct exercisepacket {
  uint16_t header[HEADER_SZ];
  char data[DATA_SZ];  // payload
};

// class to be tested. Implements the demo header structure below.
// exports routines to get and set the header parameters val (integer between 0 and 3) and payload_length (16 bit unsigned integer) for a buffer containing the packet
// this class manages the fields of : Type, Window, Seq num, Length, and CRC1
//  0 1 2 3 7 8  15 16      32
// +---+-+---+-----+--------+
// |Ty |T|Win| seq |        |
// |pe |R|dow| num |length  |
// +---+-+---+-----+--------+
// +------------------------+
// |  Timestanp(4bytes)     |
// |(not managing this time)|
// +------------------------+
// +------------------------+
// |  CRC1(4bytes)          |
// |                        |
// +------------------------+
// NOTE: this class does not allocate memory for the packet buffer. Memory for the buffer must be already allocated by the calling code.

class ExerciseHeader {
private:
  uint8_t * packet;  // address of the buffer
  int size;          // size of buffer in bytes

  // start index of the Type field (private)
  const int TP=0;

  // start index of the Window field (private)
  const int WD=0;

  // start index of the Sequence num field (private)
  const int SN=1;

  // start index of length field
  const int LG=2;

  // start index of CRC1 field
  const int C1=4;
  
public:
  // default constructor, packet buffer address set to NULL
  ExerciseHeader();

  // assigns an externally allocated buffer to the class; the set/get member functions will manage the headers fields for this buffer
  // buf = the address of the buffer
  // sz = the size in bytes of the buffer
  void setBuffer(void * buf, int sz);
  
  // sets the value of the payload length
  // val = length; if val > DATA_SZ, the value set is DATA_SZ
  // if buffer address is NULL, does nothing.
  void setLength(unsigned int val);

  // returns the length of the payload
  // if buffer address is NULL, returns 0
  unsigned int getLength() const;

  // returns the type field of the header. Must be between 0..3 since
  // it is a 2 bit value
  // if buffer address is NULL, returns 0
  unsigned int getType() const;

  // sets the type field of the header.
  // If the val is not between 0..3, the value set is 0
  // if buffer address is NULL, does nothing
  void setType(unsigned int val);

  //returns the window field of the header. must be between 0..31
  // it is a 5 bit value
  // if buffer address is NULL, returns 0
  unsigned int getWindow() const;

  // sets the type field of the header.
  // If the val is not between 0..3, the value set is 0
  // if buffer address is NULL, does nothing
  void setWindow(unsigned int val) const;

  //returns the window field of the header. must be between 0..31
  // it is a 5 bit value
  // if buffer address is NULL, returns 0
  unsigned int getSequenceNum() const;

  // sets the type field of the header.
  // If the val is not between 0..3, the value set is 0
  // if buffer address is NULL, does nothing
  void setSequenceNum(unsigned int val) const;

  void setCRC(unsigned int val);
  unsigned int getCRC() const;
  
  // returns the size of the packet in bytes, including headers and data
  // to be used with recvfrom() or sendto()
  unsigned int getBufferSize() const {
    return size;
  }

  // returns pointer to the structure holding the thePacket, including the headers
  // To be used with recvfrom or sendto
  void * getBufferAddress() {
    return packet;
  }

  // returns a pointer to the first byte of the payload from the packet buffer;
  // returns NULL if the buffer is NULL
  void * getPayloadAddress() {
    if (packet != nullptr && size > HEADER_SZ) {
      return packet+HEADER_SZ;
    } else {
      return nullptr;
    }
  }
};

#endif
