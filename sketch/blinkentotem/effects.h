
#ifndef EFFECTS_H
  #define EFFECTS_H

  #include <Adafruit_NeoPixel.h>
  #include "comm.h"

  class Spinner {
    private:
      uint16_t phase = 0;
      uint8_t frequency = 0;
      uint8_t v_min = 0;
      uint8_t v_max = 0;
      uint8_t outvalue = 0;

    public:

      void set(spin_t & spin);

      bool step(uint8_t dt);

      uint8_t lighten(uint8_t minimum);

      void clear();
  };

  class Fader {
    private:
      union {
        uint16_t whole = 0;
        struct {
          uint8_t low;
          uint8_t high;
        };
      } value;
      uint16_t decayrate;
      uint8_t outvalue = 0;

    public:

      void set(fade_t & fade);

      bool step(uint8_t dt);

      uint8_t lighten(uint8_t minimum);

      void clear();
  };

  class Flash {
    private:
      uint8_t timer = 0;
      uint8_t outvalue = 0;

    public:

      void set(uint8_t time, uint8_t brightness);

      bool step(uint8_t dt);

      uint8_t lighten(uint8_t minimum);

      void clear();
  };

  class SlowGlow {
    private:
      uint16_t timeaccumulator = 0;
      uint16_t ticklength = 100;
      uint8_t outvalue = 0;
      uint8_t targetvalue = 0;

    public:

      void setTarget(uint8_t value);

      bool step(uint8_t dt);

      uint8_t lighten(uint8_t minimum);

      void clear();
  };

#endif
