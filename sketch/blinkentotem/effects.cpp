
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

bool Spinner::active() {
  return frequency != 0;
}

void Spinner::clear() {
  phase = random(0xff00);
  frequency = 0;
  v_min = 0;
  v_max = 0;
  outvalue = 0;
}

void Fader::set(fade_t & fade) {
  if(fade.value > value.high)
    value.high = fade.value;
  decayrate = 4 * fade.decayrate;
}

bool Fader::step(uint8_t dt) {
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

bool Fader::active() {
  return value.whole != 0;
}

void Fader::clear() {
  value.whole = 0;
}

void Flash::set(uint8_t t) {
  timer = t;
}

void Flash::update(uint8_t t_min) {
  if(t_min > timer)
    timer = t_min;
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

bool Flash::active() {
  return timer != 0;
}

void Flash::clear() {
  timer = 0;
}
