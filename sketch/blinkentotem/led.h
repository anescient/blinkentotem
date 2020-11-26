
#ifndef LED_H
  #define LED_H

  #include <Adafruit_NeoPixel.h>
  #include "comm.h"
  #include "effects.h"

  class RGBLED {
    public:
      rgb_t color;

      Flash red_flash;
      Flash green_flash;

      void clearColor();

      void clearEffects();

      bool setRGB(rgb_t & rgb);

      bool step(uint8_t dt);

      void renderTo(Adafruit_NeoPixel & rgbPix, int index);
  };

  //////////////////////////////////////////////////

  class RGBWLED {
    public:
      rgbw_t color;

      Spinner blue_spin;

      Fader red_fade;
      Fader green_fade;
      Fader white_fade;

      void clearColor();

      void clearEffects();

      bool setRGBW(rgbw_t & rgbw);

      bool setRed(uint8_t red);

      bool step(uint8_t dt);

      void renderTo(Adafruit_NeoPixel & rgbwPix, int index);
  };

#endif
