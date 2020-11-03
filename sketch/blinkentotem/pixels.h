
#ifndef PIXELS_H
  #define PIXELS_H

  #include <Adafruit_NeoPixel.h>
  #include "hwconfig.h"
  #include "comm.h"

  class Pixels {
    private:
      // multiple instances of this thing almost works
      // they seem to share memory (in a bad way)
      Adafruit_NeoPixel rgbpix;
      Adafruit_NeoPixel rgbwpix;

      bool offline = false;

      class Spinner {
        private:
          uint16_t phase = 0;

        public:
          uint8_t frequency = 0;
          uint8_t b_min = 0;
          uint8_t b_max = 0;

          void step(uint8_t dt);

          bool exportBlue(rgbw_t & rgbw);

          void clear();
      };

      Spinner spinners[RGBW_COUNT];

      class Flash {
        public:
          uint16_t red = 0;
          uint16_t green = 0;

          bool step(uint8_t dt);

          void clear();
      };

      Flash raidFlash[RAID_COUNT];
      uint8_t raidRed = 30;
      uint8_t raidGreen = 70;

      Flash drumFlash[DRUM_COUNT];
      uint8_t drumRed = 150;
      uint8_t drumGreen = 200;
      bool drumFlip = false;

      uint16_t maxPulse = 1000;

      void waitForPixels();

      void addPulse(uint16_t & target, uint8_t x);

    public:
      rgb_t rgb[RGB_COUNT];
      rgbw_t rgbw[RGBW_COUNT];

      void begin();

      void setConfig(config_t & config);

      void setOffline(bool offline);

      void updateRGB(rgb_t * rgbframe);

      void updateRGB(rgb_t * rgbframe, size_t skip, size_t count);

      void updateRGBW(rgbw_t * rgbwframe);

      void updateRGBW_R(uint8_t * red);

      void updateRGBW_G(uint8_t * green);

      void updateSpins(spin_t * spins);

      void flashRaid(iopulse_t * pulses);

      void flashDrum(iopulse_t & pulse);

      void step(uint8_t dt);

      void showRGB();

      void showRGBW();

      void clear();

      void clearEffects();
  };
#endif
