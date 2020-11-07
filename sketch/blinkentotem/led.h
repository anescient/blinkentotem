
#ifndef LED_H
  #define LED_H

  #include <Adafruit_NeoPixel.h>
  #include "comm.h"
  #include "effects.h"

  class RGBLED {
    public:
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;

      Fader red_fade;
      Fader green_fade;

      Flash red_flash;
      Flash green_flash;

      void clearColor();

      void clearEffects();

      bool setRGB(rgb_t & rgb);

      bool step(uint8_t dt);

      void renderTo(Adafruit_NeoPixel & rgbPix, int index);
  };

  class RGBWLED {
    public:
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;
      uint8_t w = 0;

      Spinner blue_spin;

      Fader white_fade;
      Fader green_fade;

      SlowGlow red_glow;

      void clearColor();

      void clearEffects();

      bool setRGBW(rgbw_t & rgbw);

      bool setRed(uint8_t red);


      bool step(uint8_t dt);

      void renderTo(Adafruit_NeoPixel & rgbwPix, int index);
  };

#endif
