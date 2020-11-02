
#include "pixels.h"

void Pixels::Spinner::step(uint8_t dt) {
  if(dt > 40)
    dt = 40;
  phase += ((uint16_t)frequency) * 6 * dt;
}

bool Pixels::Spinner::exportBlue(rgbw_t & rgbw) {
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

bool Pixels::Flash::step(uint8_t dt) {
  if(dt == 0)
    return false;
  bool zeroed = false;
  if(red > 0) {
    if(red < dt)
      red = 0;
    else
      red -= dt;
    zeroed |= red == 0;
  }
  if(green > 0) {
    if(green < dt)
      green = 0;
    else
      green -= dt;
    zeroed |= green == 0;
  }
  return zeroed;
}

void Pixels::Flash::clear() {
  red = 0;
  green = 0;
}

void Pixels::waitForPixels() {
  while(!rgbpix.canShow());
  while(!rgbwpix.canShow());
}

void Pixels::addPulse(uint16_t & target, uint8_t x) {
  target += x;
  if(target > maxPulse)
    target = maxPulse;
}

void Pixels::begin() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();

  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();
}

void Pixels::setConfig(config_t & config) {
  raidRed = config.raidRed;
  raidGreen = config.raidGreen;
  maxPulse = ((uint16_t)config.maxPulse << 1);
}

void Pixels::updateRGB(rgb_t * rgbframe) {
  updateRGB(rgbframe, 0, RGB_COUNT);
}

void Pixels::updateRGB(rgb_t * rgbframe, size_t skip, size_t count) {
  for(int i = 0; i < count; i++)
    rgb[i + skip] = rgbframe[i];
}

void Pixels::updateRGBW(rgbw_t * rgbwframe) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i] = rgbwframe[i];
}

void Pixels::updateRGBW_R(uint8_t * red) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].r = red[i];
}

void Pixels::updateRGBW_G(uint8_t * green) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].g = green[i];
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

void Pixels::flashRaid(iopulse_t * pulses) {
  for(int i = 0; i < RAID_COUNT; i++) {
    iopulse_t & pulse = pulses[i];
    Flash & flash = raidFlash[i];
    addPulse(flash.green, pulse.read);
    addPulse(flash.red, pulse.write);
  }
}

void Pixels::flashDrum(iopulse_t & pulse) {
  addPulse(drumFlash[0].green, pulse.read);
  addPulse(drumFlash[1].red, pulse.write);
  drumFlip = !drumFlip;
}

void Pixels::step(uint8_t dt) {
  if(dt == 0)
    return;

  bool changed = false;
  for(int i = 0; i < RGBW_COUNT; i++) {
    Spinner & spinner = spinners[i];
    if(spinner.frequency != 0) {
      spinner.step(dt);
      changed |= spinner.exportBlue(rgbw[i]);
    }
  }
  if(changed)
    showRGBW();

  changed = false;
  for(int i = 0; i < RAID_COUNT; i++)
    changed |= raidFlash[i].step(dt);
  for(int i = 0; i < DRUM_COUNT; i++)
    changed |= drumFlash[i].step(dt);
  if(changed)
    showRGB();
}

void Pixels::showRGB() {
  waitForPixels();
  for(int i = 0; i < RGB_COUNT; i++) {
    rgb_t & c = rgb[i];
    rgbpix.setPixelColor(i, c.r, c.g, c.b);
  }

  for(int i = 0; i < RAID_COUNT; i++) {
    Flash & flash = raidFlash[i];
    rgb_t & c = rgb[RAID_OFFSET + i];
    uint8_t r = flash.red ? raidRed : c.r;
    uint8_t g = flash.green ? raidGreen : c.g;
    rgbpix.setPixelColor(RAID_OFFSET + i, r, g, c.b);
  }

  bool flashing = false;
  for(int i = 0; i < DRUM_COUNT; i++) {
    Flash & flash = drumFlash[i];
    flashing |= flash.red || flash.green;
  }
  if(flashing) {
    for(int i = 0; i < DRUM_COUNT; i++) {
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;
      Flash & flash = drumFlash[drumFlip ? 1 - i : i];
      r = flash.red ? drumRed : 0;
      g = flash.green ? drumGreen : 0;
      rgbpix.setPixelColor(DRUM_OFFSET + i, r, g, b);
    }
  }

  rgbpix.show();
}

void Pixels::showRGBW() {
  waitForPixels();
  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t c = rgbw[i];
    Spinner & spinner = spinners[i];
    if(spinner.frequency != 0)
      spinners[i].exportBlue(c);
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  rgbwpix.show();
}

void Pixels::clear() {
  memset(rgb, 0, RGB_COUNT * sizeof(rgb_t));
  memset(rgbw, 0, RGBW_COUNT * sizeof(rgbw_t));
  for(int i = 0; i < RGBW_COUNT; i++)
    spinners[i].clear();
  for(int i = 0; i < RAID_COUNT; i++)
    raidFlash[i].clear();
  for(int i = 0; i < DRUM_COUNT; i++)
    drumFlash[i].clear();
}
