
#include "pixels.h"

void Pixels::Spinner::setParams(const spin_params_t & params) {
  velocity = params.frequency;
  b_min = params.b_min;
  b_max = params.b_max;
}

void Pixels::Spinner::step(uint8_t dt) {
  phase += ((uint16_t)velocity) * dt;
}

bool Pixels::Spinner::exportrgbw(rgbw_t & rgbw) {
  uint8_t x = phase >> 8;
  uint8_t b = x < 128 ? x : 255 - x;
  if(b == rgbw.b)
    return false;
  rgbw.b = b;
  return true;
}

void Pixels::Spinner::clear() {
  velocity = 0;
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

void Pixels::setSpins(spin_params_t * spins) {
  for(int i = 0; i < RGBW_COUNT; i++)
    spinners[i].setParams(spins[i]);
}

void Pixels::step(uint8_t dt) {
  bool changed = false;
  for(int i = 0; i < RGBW_COUNT; i++) {
    Spinner & spinner = spinners[i];
    if(spinner.velocity != 0) {
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
  digitalWrite(13, HIGH);
  waitForPixels();
  rgbpix.show();
  digitalWrite(13, LOW);
}

void Pixels::showRGBW() {
  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t c = rgbw[i];
    Spinner & spinner = spinners[i];
    if(spinner.velocity != 0)
      spinners[i].exportrgbw(c);
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  digitalWrite(13, HIGH);
  waitForPixels();
  rgbwpix.show();
  digitalWrite(13, LOW);
}

void Pixels::clear() {
  memset(rgb, 0, RGB_SIZE);
  memset(rgbw, 0, RGBW_SIZE);
  for(int i = 0; i < RGBW_COUNT; i++)
    spinners[i].clear();
}
