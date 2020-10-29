
#include "comm.h"

void Comm::begin() {
  // 56k is the highest it goes without dropping bits
  Serial.begin(56000);
}

datatype_t Comm::getData(int timeoutms) {
  datatype_t datatype = NONE;
  size_t datasize = 0;
  Serial.setTimeout(timeoutms);
  if(Serial.find('$')) {
    size_t headersize = Serial.readBytesUntil('\0', buffer, COMMS_BUFFER_SIZE);
    if(headersize == 0)
      return NONE;
    switch(buffer[0]) {
      case ' ':
        datatype = PING;
        break;

      case '1':
        datatype = RGBFRAME;
        datasize = RGB_COUNT * sizeof(rgb_t);
        break;

      case '2':
        datatype = RGBWFRAME;
        datasize = RGBW_COUNT * sizeof(rgbw_t);
        break;

      case 's':
        datatype = CPU_BLUE;
        datasize = RGBW_COUNT * sizeof(spin_t);
        break;

      case 'h':
        datatype = CPU_RED;
        datasize = RGBW_COUNT;
        break;

      case 'i':
        datatype = CPU_GREEN;
        datasize = RGBW_COUNT;
        break;

      case 'l':
        datatype = LAMPS;
        datasize = 2 * sizeof(rgb_t);
        break;

      case 'd':
        datatype = DRUM;
        datasize = 2 * sizeof(rgb_t);
        break;

      case 'r':
        datatype = RAID;
        datasize = 4 * sizeof(rgb_t);
        break;
    }

    if(datasize > 0 && Serial.readBytes(buffer, datasize) != datasize) {
      datatype = NONE;
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
    }
  }
  return datatype;
}

spin_t * Comm::getSpins() {
  return ((spin_t*)buffer);
}

void Comm::exportrgb(rgb_t * dest) {
  memcpy(dest, buffer, RGB_SIZE);
}

void Comm::exportrgb(rgb_t * dest, size_t index) {
  dest[index] = ((rgb_t*)buffer)[index];
}

void Comm::exportrgbw(rgbw_t * dest) {
  memcpy(dest, buffer, RGBW_SIZE);
}

void Comm::exportrgbw(rgbw_t * dest, size_t index) {
  dest[index] = ((rgbw_t*)buffer)[index];
}

void Comm::exportrgbw_red(rgbw_t * dest) {
  for(int i = 0; i < RGBW_COUNT; i++)
    dest[i].r = buffer[i];
}

void Comm::exportrgbw_green(rgbw_t * dest) {
  for(int i = 0; i < RGBW_COUNT; i++)
    dest[i].g = buffer[i];
}
