
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

    case FLUSH:
      pixels.show();
      break;

    case CLEAR:
      pixels.clearColors();
      pixels.clearEffects();
      break;

    case CONFIG:
      pixels.setConfig(comm.buffer.config);
      break;

    case RGBFRAME:
      pixels.setRGB(comm.buffer.rgb);
      break;

    case RGBWFRAME:
      pixels.setRGBW(comm.buffer.rgbw);
      break;

    case CPU_SPIN_B:
      for(int i = 0; i < RGBW_COUNT; i++)
        pixels.rgbw[i].blue_spin.set(comm.buffer.spins[i]);
      break;

    case CPU_FADE_G:
      for(int i = 0; i < RGBW_COUNT; i++)
        pixels.rgbw[i].green_fade.set(comm.buffer.fades[i]);
      break;

    case CPU_FADE_W:
      for(int i = 0; i < RGBW_COUNT; i++)
        pixels.rgbw[i].white_fade.set(comm.buffer.fades[i]);
      break;

    case CPU_GLOW_R:
      for(int i = 0; i < RGBW_COUNT; i++)
        pixels.rgbw[i].red_glow.setTarget(comm.buffer.bytes[i]);
      break;

    case RAID:
      pixels.updateRGB(comm.buffer.rgb, RAID_OFFSET, RAID_COUNT);
      break;

    case DRUM:
      pixels.updateRGB(comm.buffer.rgb, DRUM_OFFSET, DRUM_COUNT);
      break;

    case LAMP:
      pixels.updateRGB(comm.buffer.rgb, LAMP_OFFSET, LAMP_COUNT);
      break;

    case RAIDFLASH:
      pixels.pulseRaid(comm.buffer.pulses);
      break;

    case DRUMFLASH:
      pixels.pulseDrum(comm.buffer.pulses[0]);
      break;
  }
  idleTime = 0;
}
