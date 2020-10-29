
#include "pixels.h"

void Pixels::Spinner::step(uint8_t dt) {
  if(dt > 40)
    dt = 40;
  phase += ((uint16_t)frequency) * 6 * dt;
}

bool Pixels::Spinner::exportrgbw(rgbw_t & rgbw) {
  uint16_t x = phase >> 7;
  x = x < 256 ? x : 511 - x; // sawtooth
  x = b_min + ((x * (b_max - b_min)) >> 8);
  uint8_t b = Adafruit_NeoPixel::gamma8(x);
  if(x > 0 && b == 0)
    b = 1;
  if(b == rgbw.b)
    return false;
  rgbw.b = b;
  return true;
}

void Pixels::Spinner::clear() {
  frequency = 0;
  b_min = 0;
  b_max = 0;
}

void Pixels::waitForPixels() {
  while(!rgbpix.canShow());
  while(!rgbwpix.canShow());
}

void Pixels::begin() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();

  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();
}

void Pixels::updateRGB(rgb_t * rgbframe) {
  for(int i = 0; i < RGB_COUNT; i++)
    rgb[i] = rgbframe[i];
}

void Pixels::updateRGBW(rgbw_t * rgbwframe) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i] = rgbwframe[i];
}

void Pixels::updateSpins(spin_t * spins) {
  for(int i = 0; i < RGBW_COUNT; i++) {
    spin_t & s = spins[i];
    Spinner & spnr = spinners[i];
    spnr.frequency = s.frequency;
    spnr.b_max = s.brightness;
    spnr.b_min = s.brightness / 4;
    if(spnr.b_min < 1)
      spnr.b_min = 1;
    if(spnr.b_max < spnr.b_min)
      spnr.b_max = spnr.b_min;
  }
}

void Pixels::step(uint8_t dt) {
  bool changed = false;
  for(int i = 0; i < RGBW_COUNT; i++) {
    Spinner & spinner = spinners[i];
    if(spinner.frequency != 0) {
      spinner.step(dt);
      changed |= spinner.exportrgbw(rgbw[i]);
    }
  }
  if(changed)
    showRGBW();
}

void Pixels::showRGB() {
  for(int i = 0; i < RGB_COUNT; i++) {
    rgb_t & c = rgb[i];
    rgbpix.setPixelColor(i, c.r, c.g, c.b);
  }
  waitForPixels();
  rgbpix.show();
}

void Pixels::showRGBW() {
  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t c = rgbw[i];
    Spinner & spinner = spinners[i];
    if(spinner.frequency != 0)
      spinners[i].exportrgbw(c);
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  waitForPixels();
  rgbwpix.show();
}

void Pixels::clear() {
  memset(rgb, 0, RGB_COUNT * sizeof(rgb_t));
  memset(rgbw, 0, RGBW_COUNT * sizeof(rgbw_t));
  for(int i = 0; i < RGBW_COUNT; i++)
    spinners[i].clear();
}
