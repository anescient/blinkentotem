
#ifndef PIXELS_H
  #define PIXELS_H

  #include <Adafruit_NeoPixel.h>
  #include "hwconfig.h"
  #include "comm.h"

  class Pixels {
    private:
      // multiple instances of this thing kinda works
      // they seem to share memory
      Adafruit_NeoPixel rgbpix;
      Adafruit_NeoPixel rgbwpix;

      class Spinner {
        private:
          uint16_t phase;

        public:
          uint8_t frequency;
          uint8_t b_min;
          uint8_t b_max;

          void step(uint8_t dt);

          bool exportrgbw(rgbw_t & rgbw);

          void clear();
      };

      Spinner spinners[RGBW_COUNT];

      void waitForPixels();

    public:
      rgb_t rgb[RGB_COUNT];
      rgbw_t rgbw[RGBW_COUNT];

      void begin();

      void updateSpins(spin_t * spins);

      void step(uint8_t dt);

      void showRGB();

      void showRGBW();

      void clear();
  };
#endif
