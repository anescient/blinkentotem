
#include "pixels.h"

void Pixels::Spinner::update(spin_t & spin) {
    frequency = spin.frequency;
    v_max = spin.brightness;
    v_min = spin.brightness / 8;
    if(v_min < 1)
      v_min = 1;
    if(v_max < v_min)
      v_max = v_min;
}

bool Pixels::Spinner::step(uint8_t dt) {
  if(dt > 40)
    dt = 40;
  phase += ((uint16_t)frequency) * 5 * dt;
  uint16_t x = phase >> 7;
  x = Adafruit_NeoPixel::sine8(x);
  x = v_min + ((x * (v_max - v_min)) >> 8);
  uint8_t v = Adafruit_NeoPixel::gamma8(x);
  if(v == 0)
    v = 1;
  if(v == outvalue)
    return false;
  outvalue = v;
  return true;
}

bool Pixels::Spinner::active() {
  return frequency != 0;
}

void Pixels::Spinner::clear() {
  phase = random(0xff00);
  frequency = 0;
  v_min = 0;
  v_max = 0;
  outvalue = 0;
}

void Pixels::Fader::update(fade_t & fade) {
  if(fade.value > value.high)
    value.high = fade.value;
  decayrate = 4 * fade.decayrate;
}

bool Pixels::Fader::step(uint8_t dt) {
  if(outvalue == 0 && value.whole == 0)
    return false;
  uint16_t delta = dt * decayrate;
  if(value.whole < delta)
    value.whole = 0;
  else
    value.whole -= delta;
  uint8_t v = Adafruit_NeoPixel::gamma8(value.high);
  if(v != outvalue) {
    outvalue = v;
    return true;
  }
  return false;
}

bool Pixels::Fader::active() {
  return value.whole != 0;
}

void Pixels::Fader::clear() {
  value.whole = 0;
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

bool Pixels::Flash::active() {
  return red || green;
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

  clear();
  clearEffects();
}

void Pixels::setConfig(config_t & config) {
  maxPulse = ((uint16_t)config.maxPulse << 1);
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

void Pixels::updateRGBW_W(uint8_t * white) {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbw[i].w = white[i];
}

void Pixels::updateSpins_B(spin_t * spins) {
  for(int i = 0; i < RGBW_COUNT; i++)
    spins_blue[i].update(spins[i]);
}

void Pixels::updateSpins_W(spin_t * spins) {
  for(int i = 0; i < RGBW_COUNT; i++)
    spins_white[i].update(spins[i]);
}

void Pixels::updateFades_W(fade_t * fades) {
  for(int i = 0; i < RGBW_COUNT; i++)
    fades_white[i].update(fades[i]);
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
    Spinner & spin_b = spins_blue[i];
    if(spin_b.active())
      changed |= spin_b.step(dt);
    Spinner & spin_w = spins_white[i];
    if(spin_w.active())
      changed |= spin_w.step(dt);
    Fader & fade_w = fades_white[i];
    if(fade_w.active())
      changed |= fade_w.step(dt);
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
  if(offline) {
    rgbpix.clear();
    rgbpix.setPixelColor(0, 20, 0, 0);
    rgbpix.setPixelColor(1, 20, 0, 0);
    rgbpix.show();
    return;
  }

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
  for(int i = 0; i < DRUM_COUNT; i++)
    flashing |= drumFlash[i].active();
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
  if(offline) {
    rgbwpix.clear();
    rgbwpix.show();
    return;
  }

  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t c = rgbw[i];
    Spinner & spin_b = spins_blue[i];
    if(spin_b.active())
      c.b = spin_b.outvalue;
    Spinner & spin_w = spins_white[i];
    if(spin_w.active())
      c.w = spin_w.outvalue;
    Fader & fade_w = fades_white[i];
    if(fade_w.active() && fade_w.outvalue > c.w)
      c.w = fade_w.outvalue;
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  rgbwpix.show();
}

void Pixels::clear() {
  memset(rgb, 0, RGB_COUNT * sizeof(rgb_t));
  memset(rgbw, 0, RGBW_COUNT * sizeof(rgbw_t));
  clearEffects();
}

void Pixels::clearEffects() {
  for(int i = 0; i < RGBW_COUNT; i++) {
    spins_blue[i].clear();
    spins_white[i].clear();
    fades_white[i].clear();
  }
  for(int i = 0; i < RAID_COUNT; i++)
    raidFlash[i].clear();
  for(int i = 0; i < DRUM_COUNT; i++)
    drumFlash[i].clear();
}
