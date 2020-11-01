
#include "comm.h"
#include "pixels.h"

Comm comm;
Pixels pixels;

uint16_t idlecycles;
unsigned long lastms;

void offline() {
  pixels.clear();
  pixels.showRGBW();
  pixels.rgb[0].r = 20;
  pixels.rgb[1].r = 20;
  pixels.showRGB();
}

void setup() {
  idlecycles = 0;
  lastms = millis();

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pixels.begin();
  offline();

  comm.begin();
}

void loop() {
  unsigned long ms = millis();
  if(ms < lastms) { // overflow
    lastms = ms;
    return;
  }

  unsigned long dt = ms - lastms;
  lastms = ms;
  uint8_t dt8 = dt > 255 ? 255 : dt;

  if(idlecycles > 200) {
    offline();
    idlecycles = 0;
    return;
  }

  switch(comm.receive(5)) {
    default:
    case NONE:
      idlecycles++;
      pixels.step(dt8);
      return;

    case PING:
      break;

    case CONFIG:
      pixels.setConfig(comm.buffer.config);
      break;

    case RGBFRAME:
      pixels.updateRGB(comm.buffer.rgb);
      pixels.showRGB();
      break;

    case RGBWFRAME:
      pixels.updateRGBW(comm.buffer.rgbw);
      pixels.showRGBW();
      break;

    case CPU_SPIN:
      pixels.updateSpins(comm.buffer.spins);
      pixels.showRGBW();
      break;

    case CPU_RED:
      pixels.updateRGBW_R(comm.buffer.bytes);
      pixels.showRGBW();
      break;

    case CPU_GREEN:
      pixels.updateRGBW_G(comm.buffer.bytes);
      pixels.showRGBW();
      break;

    case RAID:
      pixels.updateRGB(comm.buffer.rgb, RAID_OFFSET, RAID_COUNT);
      pixels.showRGB();
      break;

    case DRUM:
      pixels.updateRGB(comm.buffer.rgb, 2, 2);
      pixels.showRGB();
      break;

    case LAMPS:
      pixels.updateRGB(comm.buffer.rgb, 0, 2);
      pixels.showRGB();
      break;

    case RAIDPULSE:
      pixels.updateRaid(comm.buffer.raidpulse);
      pixels.showRGB();
      break;
  }
  idlecycles = 0;
}
