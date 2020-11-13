
#include "comm.h"

void Comm::begin() {
  // 56k is the highest it goes without dropping bits
  Serial.begin(56000);
}

datatype_t Comm::receive(int timeoutms) {
  Serial.setTimeout(timeoutms);
  if(!Serial.find('$'))
    return NONE;
  Serial.setTimeout(500);

  char command;
  if(!Serial.readBytes(&command, 1))
    return ERROR;

  datatype_t datatype = ERROR;
  size_t datasize = 0;
  switch(command) {
    default:
      break;

    case ' ':
      datatype = PING;
      break;

    case ';':
      datatype = FLUSH;
      break;

    case 'c':
      datatype = CLEAR;
      break;

    case 'p':
      datatype = CONFIG;
      datasize = sizeof(config_t);
      break;

    case '1':
      datatype = RGBFRAME;
      datasize = RGB_COUNT * sizeof(rgb_t);
      break;

    case 'd':
      datatype = DRUM;
      datasize = DRUM_COUNT * sizeof(rgb_t);
      break;

    case 'r':
      datatype = RAID;
      datasize = RAID_COUNT * sizeof(rgb_t);
      break;

    case 'l':
      datatype = LAMP;
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

    case '2':
      datatype = RGBWFRAME;
      datasize = RGBW_COUNT * sizeof(rgbw_t);
      break;

    case 'g':
      datatype = RGBW_FADE_R;
      datasize = RGBW_COUNT * sizeof(fade_t);
      break;

    case 'i':
      datatype = RGBW_FADE_G;
      datasize = RGBW_COUNT * sizeof(fade_t);
      break;

    case 's':
      datatype = RGBW_SPIN_B;
      datasize = RGBW_COUNT * sizeof(spin_t);
      break;

    case 'z':
      datatype = RGBW_FADE_W;
      datasize = RGBW_COUNT * sizeof(fade_t);
      break;
  }

  if(datasize > 0 && Serial.readBytes(buffer.bytes, datasize) != datasize) {
    datatype = ERROR;
    digitalWrite(13, HIGH);
  }
  return datatype;
}
