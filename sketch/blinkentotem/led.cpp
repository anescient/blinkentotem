
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
  if(dt == 0)
    return false;
  bool changed = false;
  changed |= red_fade.step(dt);
  changed |= green_fade.step(dt);
  changed |= red_flash.step(dt);
  changed |= green_flash.step(dt);
  return changed;
}

void RGBLED::renderTo(Adafruit_NeoPixel & rgbPix, int index) {
  uint8_t rr = r;
  uint8_t gg = g;
  if(red_fade.active())
    rr = red_fade.outvalue;
  if(green_fade.active())
    gg = green_fade.outvalue;
  if(red_flash.active())
    rr = red_flash.brightness;
  if(green_flash.active())
    gg = green_flash.brightness;
  rgbPix.setPixelColor(index, rr, gg, b);
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

bool RGBWLED::step(uint8_t dt) {
  if(dt == 0)
    return false;
  bool changed = false;
  changed |= blue_spin.step(dt);
  changed |= white_fade.step(dt);
  return changed;
}

void RGBWLED::renderTo(Adafruit_NeoPixel & rgbwPix, int index) {
  uint8_t bb = b;
  uint8_t ww = w;
  if(blue_spin.active())
    bb = blue_spin.outvalue;
  if(white_fade.active())
    ww = white_fade.outvalue;
  rgbwPix.setPixelColor(index, g, r, bb, ww);
}
