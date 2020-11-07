
#include "led.h"

void RGBLED::clearColor() {
  r = 0;
  g = 0;
  b = 0;
}

void RGBLED::clearEffects() {
  red_fade.clear();
  green_fade.clear();
  red_flash.clear();
  green_flash.clear();
}

bool RGBLED::setRGB(rgb_t & rgb) {
  if(rgb.r == r && rgb.g == g && rgb.b == b)
    return false;
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
  return true;
}

bool RGBLED::step(uint8_t dt) {
  bool changed = false;
  changed |= red_fade.step(dt);
  changed |= green_fade.step(dt);
  changed |= red_flash.step(dt);
  changed |= green_flash.step(dt);
  return changed;
}

void RGBLED::renderTo(Adafruit_NeoPixel & rgbPix, int index) {
  rgbPix.setPixelColor(index,
    red_fade.lighten(red_flash.lighten(r)),
    green_fade.lighten(green_flash.lighten(g)), b);
}

////////////////////////////////////////////////////////////////////

void RGBWLED::clearColor() {
  r = 0;
  g = 0;
  b = 0;
  w = 0;
}

void RGBWLED::clearEffects() {
  blue_spin.clear();
  white_fade.clear();
  green_fade.clear();
}

bool RGBWLED::setRGBW(rgbw_t & rgbw) {
  if(rgbw.r == r && rgbw.g == g && rgbw.b == b && rgbw.w == w)
    return false;
  r = rgbw.r;
  g = rgbw.g;
  b = rgbw.b;
  w = rgbw.w;
  return true;
}

bool RGBWLED::setRed(uint8_t red) {
  if(r == red)
    return false;
  r = red;
  return true;
}

bool RGBWLED::step(uint8_t dt) {
  bool changed = false;
  changed |= blue_spin.step(dt);
  changed |= white_fade.step(dt);
  changed |= green_fade.step(dt);
  return changed;
}

void RGBWLED::renderTo(Adafruit_NeoPixel & rgbwPix, int index) {
  rgbwPix.setPixelColor(index,
    green_fade.lighten(g), r,
    blue_spin.lighten(b),
    white_fade.lighten(w));
}
