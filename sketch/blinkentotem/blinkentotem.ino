
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

  if(idlecycles > 400) {
    offline();
    idlecycles = 0;
    return;
  }

  switch(comm.receive(10)) {
    default:
    case NONE:
      idlecycles++;
      pixels.step(dt8);
      return;

    case PING:
      break;

    case RGBFRAME:
      pixels.updateRGB(comm.buffer.rgb);
      pixels.showRGB();
      break;

    case RGBWFRAME:
      pixels.updateRGBW(comm.buffer.rgbw);
      pixels.showRGBW();
      break;

    case CPU_BLUE:
      pixels.updateSpins(comm.buffer.spins);
      pixels.showRGBW();
      break;

    case CPU_RED:
      break;

    case CPU_GREEN:
      break;

    case LAMPS:
      break;

    case DRUM:
      break;

    case RAID:
      break;
  }
  idlecycles = 0;
}
