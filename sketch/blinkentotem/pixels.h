
#ifndef PIXELS_H
  #define PIXELS_H

  #include <Adafruit_NeoPixel.h>
  #include "hwconfig.h"
  #include "comm.h"

  class Pixels {
    private:
      Adafruit_NeoPixel rgbpix;
      Adafruit_NeoPixel rgbwpix;

    public:
      rgb_t rgb[RGB_COUNT];
      rgbw_t rgbw[RGBW_COUNT];

      void begin();

      void showRGB();

      void showRGBW();

      void clear();
  };
#endif
