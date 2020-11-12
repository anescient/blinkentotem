
#include "effects.h"

void Spinner::set(spin_t & spin) {
  frequency = spin.frequency;
  v_max = spin.brightness;
  v_min = spin.brightness / 8;
  if(v_min < 1)
    v_min = 1;
  if(v_max < v_min)
    v_max = v_min;
}

bool Spinner::step(uint8_t dt) {
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

uint8_t Spinner::lighten(uint8_t minimum) {
  return frequency == 0 ? minimum : max(minimum, outvalue);
}

void Spinner::clear() {
  phase = random(0xff00);
  frequency = 0;
  v_min = 0;
  v_max = 0;
  outvalue = 0;
}

//////////////////////////////////////////////////

bool Fader::enabled() {
  return uprate != 0 || downrate != 0;
}

void Fader::set(fade_t & fade) {
  targetvalue = fade.targetvalue;
  uprate = fade.uprate;
  downrate = fade.downrate;
}

void Fader::updateTarget(uint8_t value) {
  targetvalue = value;
}

bool Fader::step(uint8_t dt) {
  if(value.high == targetvalue || !enabled())
    return false;
  timeaccumulator += dt;
  uint8_t timedivide = 100;
  if(timeaccumulator < timedivide)
    return false;
  dt = timedivide;
  timeaccumulator -= dt;

  uint8_t oldhigh = value.high;

  if(value.high > targetvalue) {
    if(downrate == 0xff)
      value.high = targetvalue;
    else {
      uint16_t tick = (uint16_t)dt * downrate;
      value.whole = value.whole > tick ? value.whole - tick : 0;
    }
  }

  if(value.high < targetvalue) {
    if(uprate == 0xff)
      value.high = targetvalue;
    else {
      uint16_t tick = (uint16_t)dt * uprate;
      value.whole = value.whole < (0xffff - tick) ? value.whole + tick : 0xffff;
    }
  }

  return value.high != oldhigh;
}

uint8_t Fader::lighten(uint8_t minimum) {
  return enabled() ? max(minimum, Adafruit_NeoPixel::gamma8(value.high)) : minimum;
}

void Fader::clear() {
  value.whole = 0;
  uprate = 0;
  downrate = 0;
}

//////////////////////////////////////////////////

void Flash::set(uint8_t time, uint8_t brightness) {
  timer = time;
  outvalue = brightness;
}

bool Flash::step(uint8_t dt) {
  if(dt == 0)
    return false;
  if(timer > 0) {
    timer = timer > dt ? timer - dt : 0;
    return true;
  }
  return false;
}

uint8_t Flash::lighten(uint8_t minimum) {
  return timer == 0 ? minimum : max(minimum, outvalue);
}

void Flash::clear() {
  timer = 0;
}
