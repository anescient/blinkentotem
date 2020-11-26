
#include "led.h"

void RGBLED::clearColor() {
  memset(&color, 0, sizeof(rgb_t));
}

void RGBLED::clearEffects() {
  red_flash.clear();
  green_flash.clear();
}

bool RGBLED::setRGB(rgb_t & rgb) {
  if(memcmp(&color, &rgb, sizeof(rgb_t)) == 0)
    return false;
  color = rgb;
  return true;
}

bool RGBLED::step(uint8_t dt) {
  bool changed = false;
  changed |= red_flash.step(dt);
  changed |= green_flash.step(dt);
  return changed;
}

void RGBLED::renderTo(Adafruit_NeoPixel & rgbPix, int index) {
  rgbPix.setPixelColor(index,
    red_flash.lighten(color.r),
    green_flash.lighten(color.g),
    color.b);
}

//////////////////////////////////////////////////

void RGBWLED::clearColor() {
  memset(&color, 0, sizeof(rgbw_t));
}

void RGBWLED::clearEffects() {
  red_fade.clear();
  green_fade.clear();
  blue_spin.clear();
  white_fade.clear();
}

bool RGBWLED::setRGBW(rgbw_t & rgbw) {
  if(memcmp(&color, &rgbw, sizeof(rgbw_t)) == 0)
    return false;
  color = rgbw;
  return true;
}

bool RGBWLED::step(uint8_t dt) {
  bool changed = false;
  changed |= red_fade.step(dt);
  changed |= green_fade.step(dt);
  changed |= blue_spin.step(dt);
  changed |= white_fade.step(dt);
  return changed;
}

void RGBWLED::renderTo(Adafruit_NeoPixel & rgbwPix, int index) {
  rgbwPix.setPixelColor(index,
    green_fade.lighten(color.g),
    red_fade.lighten(color.r),
    blue_spin.lighten(color.b),
    white_fade.lighten(color.w));
}
