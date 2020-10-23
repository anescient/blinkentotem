
#include "hwconfig.h"
#include "comm.h"
#include "pixels.h"

Comm comm;
Pixels pixels;

unsigned int idlecycles;

void offline() {
  pixels.clear();
  pixels.showRGBW();
  pixels.rgb[0].r = 20;
  pixels.rgb[1].r = 20;
  pixels.showRGB();
}

void setup() {
  idlecycles = 0;

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pixels.begin();
  offline();

  comm.begin(10);
}

void loop() {
  if(idlecycles > 100) {
    offline();
    idlecycles = 0;
    return;
  }

  switch(comm.getData()) {
    default:
    case NONE:
      idlecycles++;
      return;

    case RGBFRAME:
      comm.exportrgb(pixels.rgb);
      pixels.showRGB();
      break;

    case RGBWFRAME:
      comm.exportrgbw(pixels.rgbw);
      pixels.showRGBW();
      break;
  }
  idlecycles = 0;
}
