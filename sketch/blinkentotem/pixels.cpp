
#include "pixels.h"

void Pixels::begin() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();

  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();
}

void Pixels::showRGB() {
  for(int i = 0; i < RGB_COUNT; i++) {
    rgb_t & c = rgb[i];
    rgbpix.setPixelColor(i, c.r, c.g, c.b);
  }
  digitalWrite(13, HIGH);
  rgbpix.show();
  digitalWrite(13, LOW);
}

void Pixels::showRGBW() {
  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t & c = rgbw[i];
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  digitalWrite(13, HIGH);
  rgbwpix.show();
  digitalWrite(13, LOW);
}

void Pixels::clear() {
  memset(rgb, 0, RGB_SIZE);
  memset(rgbw, 0, RGBW_SIZE);
}
