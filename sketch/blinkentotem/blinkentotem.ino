
#include "comm.h"
#include "pixels.h"

Comm comm;
Pixels pixels;

unsigned long idleTime;
unsigned long lastms;

void setup() {
  idleTime = 0;
  lastms = millis();

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pixels.begin();
  pixels.setOffline(true);

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
  idleTime += dt8;
  pixels.setOffline(idleTime > 1000);
  if(idleTime > 4000)
    idleTime = 4000;

  switch(comm.receive(5)) {
    default:
    case NONE:
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
      pixels.updateRGB(comm.buffer.rgb, DRUM_OFFSET, DRUM_COUNT);
      pixels.showRGB();
      break;

    case LAMPS:
      pixels.updateRGB(comm.buffer.rgb, LAMP_OFFSET, LAMP_COUNT);
      pixels.showRGB();
      break;

    case RAIDFLASH:
      pixels.flashRaid(comm.buffer.pulses);
      pixels.showRGB();
      break;

    case DRUMFLASH:
      pixels.flashDrum(comm.buffer.pulses[0]);
      pixels.showRGB();
      break;
  }
  idleTime = 0;
}
