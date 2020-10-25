
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
          char velocity;
          char b_min;
          char b_max;

          void setParams(const spin_params_t & params);

          void step(char dt);

          bool exportrgbw(rgbw_t & rgbw);

          void clear();
      };

      Spinner spinners[RGBW_COUNT];

      void waitForPixels();

    public:
      rgb_t rgb[RGB_COUNT];
      rgbw_t rgbw[RGBW_COUNT];

      void begin();

      void setSpins(spin_params_t * spins);

      void step(char dt);

      void showRGB();

      void showRGBW();

      void clear();
  };
#endif
