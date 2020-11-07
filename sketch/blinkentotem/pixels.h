
#ifndef PIXELS_H
  #define PIXELS_H

  #include <Adafruit_NeoPixel.h>
  #include "hwconfig.h"
  #include "comm.h"
  #include "led.h"

  class Pixels {
    private:
      // multiple instances of this thing almost works
      // they seem to share memory (in a bad way)
      Adafruit_NeoPixel rgbpix;
      Adafruit_NeoPixel rgbwpix;

      bool offline = false;

      uint8_t raidRed = 30;
      uint8_t raidGreen = 70;

      uint8_t drumRed = 150;
      uint8_t drumGreen = 200;
      uint8_t drumFlip = 0;

      bool rgbDirty = true;
      bool rgbwDirty = true;

      void waitForPixels();
      void showRGB();
      void showRGBW();

    public:

      RGBLED rgb[RGB_COUNT];
      RGBWLED rgbw[RGBW_COUNT];

      void begin();

      void setConfig(config_t & config);

      void setOffline(bool offline);

      void step(uint8_t dt);


      void clearColors();

      void clearEffects();

      void setRGB(rgb_t * rgbframe);

      void setRGBW(rgbw_t * rgbwframe);

      void updateRGB(rgb_t * rgbframe, size_t index, size_t count);


      void pulseRaid(iopulse_t * pulses);

      void pulseDrum(iopulse_t & pulse);
  };

#endif
