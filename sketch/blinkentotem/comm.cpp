
#include "comm.h"

void Comm::begin() {
  // 56k is the highest it goes without dropping bits
  Serial.begin(56000);
}

datatype_t Comm::getData(int timeoutms) {
  Serial.setTimeout(timeoutms);
  if(Serial.find('$')) {
    int headersize = Serial.readBytesUntil('\0', buffer, COMMS_BUFFER_SIZE);
    if(headersize == 0)
      return NONE;
    switch(buffer[0]) {
      case '1':
        if(Serial.readBytes(buffer, RGB_SIZE) == RGB_SIZE)
          return RGBFRAME;

      case '2':
        if(Serial.readBytes(buffer, RGBW_SIZE) == RGBW_SIZE)
          return RGBWFRAME;
    }
  }
  return NONE;
}

void Comm::exportrgb(rgb_t * dest) {
  memcpy(dest, buffer, RGB_SIZE);
}

void Comm::exportrgbw(rgbw_t * dest) {
  memcpy(dest, buffer, RGBW_SIZE);
}
