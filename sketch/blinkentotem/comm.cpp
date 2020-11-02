
#include "comm.h"

void Comm::begin() {
  // 56k is the highest it goes without dropping bits
  Serial.begin(56000);
}

datatype_t Comm::receive(int timeoutms) {
  datatype_t datatype = NONE;
  size_t datasize = 0;
  Serial.setTimeout(timeoutms);
  if(Serial.find('$')) {
    size_t headersize = Serial.readBytesUntil('\0', buffer.bytes, COMMS_BUFFER_SIZE);
    if(headersize == 0)
      return NONE;
    switch(buffer.command) {
      case ' ':
        datatype = PING;
        break;

      case 'p':
        datatype = CONFIG;
        datasize = sizeof(config_t);
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
        datatype = CPU_SPIN;
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

      case 'r':
        datatype = RAID;
        datasize = RAID_COUNT * sizeof(rgb_t);
        break;

      case 'd':
        datatype = DRUM;
        datasize = DRUM_COUNT * sizeof(rgb_t);
        break;

      case 'l':
        datatype = LAMPS;
        datasize = LAMP_COUNT * sizeof(rgb_t);
        break;

      case 'f':
        datatype = RAIDFLASH;
        datasize = RAID_COUNT * sizeof(iopulse_t);
        break;

      case 'm':
        datatype = DRUMFLASH;
        datasize = sizeof(iopulse_t);
        break;

    }

    if(datasize > 0 && Serial.readBytes(buffer.bytes, datasize) != datasize) {
      datatype = NONE;
      digitalWrite(13, HIGH);
    }
  }
  return datatype;
}
