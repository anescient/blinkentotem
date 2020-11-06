
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

    public:
      uint8_t outvalue = 0;

      void set(spin_t & spin);

      bool step(uint8_t dt);

      bool active();

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

    public:
      uint8_t outvalue = 0;

      void set(fade_t & fade);

      bool step(uint8_t dt);

      bool active();

      void clear();
  };

  class Flash {
    private:
      uint8_t timer = 0;

    public:
      uint8_t brightness = 0;

      void set(uint8_t t);

      void update(uint8_t t_min);

      bool step(uint8_t dt);

      bool active();

      void clear();
  };

#endif
