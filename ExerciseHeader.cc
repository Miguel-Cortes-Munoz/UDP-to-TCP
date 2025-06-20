#include "ExerciseHeader.h"
#include <stdexcept>
#include <cstring>

ExerciseHeader::ExerciseHeader() {
  // silly code, you can do better
  packet = nullptr;
  size = 0;
}

void ExerciseHeader::setBuffer(void * buf, int sz) {
  packet = static_cast<uint8_t*> (buf);
  size = sz;
  memset(packet,0,size);
}

void ExerciseHeader::setLength(unsigned int val) {
  if (packet != nullptr && size > HEADER_SZ) {
    packet[LG] = (val>>8); // shift the integer to right by 8 bits to get the msb
    packet[LG+1] = (val&255); // bitwise AND with 8 LSB bits set to 1.
  }
}

unsigned int ExerciseHeader::getLength() const {
  if (packet != nullptr && size > HEADER_SZ) {
    return packet[LG+1] | (packet[LG]<<8);
  }
  else {
    return 0;
  }
}

unsigned int ExerciseHeader::getType() const {
  if (packet != nullptr && size > HEADER_SZ) {
    return (packet[TP] >> 6);
  }
  else {
    return 0;
  }
}
 
void ExerciseHeader::setType(unsigned int val) {
    if (packet != nullptr && size > HEADER_SZ) {
    //val must be the size of 0...3, if grether than 3 or less than 0 , val is set 0
    if(val > 3 || val < 0){
      val = 0;
    }
    // clear the val first
    packet[TP] &= 0x3f;

    // set the lowest 2 bits of val to the header field, but do not disturb the other bits
    packet[TP] |= (val << 6);
    }
}

unsigned int ExerciseHeader::getWindow() const {
  if (packet != nullptr && size > HEADER_SZ) {
    return (packet[WD] & 0x1F);
  }
  else {
    return 0;
  }
}

void ExerciseHeader::setWindow(unsigned int val) const{
    if (packet != nullptr && size > HEADER_SZ) {
    //val must be the size of 0...3, if grether than 3 or less than 0 , val is set 0
    if(val > 31 || val < 0){
      val = 0;
    }
    // clear the val first
    packet[WD] &= 0xE0;

    // set the lowest 5 bits of val to the header field, but do not disturb the other bits
    packet[WD] |= val;
    }
}

void ExerciseHeader::setSequenceNum(unsigned int val) const{
  if (packet != nullptr && size > HEADER_SZ) {
    packet[SN] = val;   
  }
}

unsigned int ExerciseHeader::getSequenceNum() const {
  if (packet != nullptr && size > HEADER_SZ) {
    return packet[SN];
  }
  else {
    return 0;
  }
}

void ExerciseHeader::setCRC(unsigned int val) {
  if (packet != nullptr && size > HEADER_SZ) {
    packet[C1] = (val>>24) &0xFF; // shift the integer to right by 24 bits to get the msb
    packet[C1+1] = (val >> 16) & 0xFF;
    packet[C1+2] = (val >> 8) & 0xFF;
    packet[C1+3] = (val & 0xFF);
  }
}

unsigned int ExerciseHeader::getCRC() const {
  if (packet != nullptr && size > HEADER_SZ) {
    return packet[C1+3] | (packet[C1+2] << 8) | (packet[C1+1]<<16) | (packet[C1]<<24);
  }
  else {
    return 0;
  }
}