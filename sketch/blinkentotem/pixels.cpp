
#include "pixels.h"

void Pixels::waitForPixels() {
  while(!rgbpix.canShow());
  while(!rgbwpix.canShow());
}

void Pixels::showRGB() {
  waitForPixels();
  rgbpix.clear();
  if(offline) {
    rgbpix.setPixelColor(0, 20, 0, 0);
    rgbpix.setPixelColor(1, 20, 0, 0);
    rgbpix.show();
    return;
  }
  for(int i = 0; i < RGB_COUNT; i++)
    rgb[i].renderTo(rgbpix, i);
  rgbpix.show();
  rgbDirty = false;
}

void Pixels::showRGBW() {
  waitForPixels();
  rgbwpix.clear();
  if(offline) {
    rgbwpix.show();
    return;
  }
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].renderTo(rgbwpix, i);
  rgbwpix.show();
  rgbwDirty = false;
}

void Pixels::begin() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();

  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();

  clearColors();
  clearEffects();
  showRGB();
  showRGBW();
}

void Pixels::setConfig(config_t & config) {
  raidRed = config.raidRed;
  raidGreen = config.raidGreen;
  drumRed = config.drumRed;
  drumGreen = config.drumGreen;
}

void Pixels::setOffline(bool offline) {
  bool changed = offline == !this->offline;
  this->offline = offline;
  if(changed) {
    if(offline) {
      clearEffects();
    }
    showRGB();
    showRGBW();
  }
}

void Pixels::step(uint8_t dt) {
  for(int i = 0; i < RGB_COUNT; i++)
    rgbDirty |= rgb[i].step(dt);
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbwDirty |= rgbw[i].step(dt);
  if(rgbDirty)
    showRGB();
  if(rgbwDirty)
    showRGBW();
}

void Pixels::clearColors() {
  for(int i = 0; i < RGB_COUNT; i++)
    rgb[i].clearColor();
  rgbDirty = true;
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].clearColor();
  rgbwDirty = true;
}

void Pixels::clearEffects() {
  for(int i = 0; i < RGB_COUNT; i++)
    rgb[i].clearEffects();
  rgbDirty = true;
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].clearEffects();
  rgbwDirty = true;
}

void Pixels::setRGB(rgb_t * rgbframe) {
  updateRGB(rgbframe, 0, RGB_COUNT);
}

void Pixels::setRGBW(rgbw_t * rgbwframe) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbwDirty |= rgbw[i].setRGBW(rgbwframe[i]);
}

void Pixels::updateRGB(rgb_t * rgbframe, size_t index, size_t count) {
  for(int i = 0; i < count; i++)
    rgbDirty |= rgb[index + i].setRGB(rgbframe[i]);
}

void Pixels::setCPU_R(uint8_t * red) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbwDirty |= rgbw[i].setRed(red[i]);
}

void Pixels::setCPU_G(uint8_t * green) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbwDirty |= rgbw[i].setGreen(green[i]);
}

void Pixels::setCPU_bluespins(spin_t * spins) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].blue_spin.set(spins[i]);
}

void Pixels::setCPU_whitefades(fade_t * fades) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].white_fade.set(fades[i]);
}

void Pixels::pulseRaid(iopulse_t * pulses) {
  for(int i = 0; i < RAID_COUNT; i++) {
    RGBLED & led = rgb[RAID_OFFSET + i];
    led.red_flash.brightness = raidRed;
    led.red_flash.update(pulses[i].write);
    led.green_flash.brightness = raidGreen;
    led.green_flash.update(pulses[i].read);
  }
  rgbDirty = true;
}

void Pixels::pulseDrum(iopulse_t & pulse) {
  RGBLED & led_a = rgb[DRUM_OFFSET + drumFlip];
  drumFlip = 1 - drumFlip;
  RGBLED & led_b = rgb[DRUM_OFFSET + drumFlip];
  led_a.green_flash.brightness = drumGreen;
  led_a.green_flash.update(pulse.read);
  led_b.red_flash.brightness = drumRed;
  led_b.red_flash.update(pulse.write);
  rgbDirty = true;
}
