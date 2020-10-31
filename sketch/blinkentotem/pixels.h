
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

      class RaidFlash {
        public:
          uint8_t red = 0;
          uint8_t green = 0;

          bool step();

          void clear();
      };

      RaidFlash raidFlash[RAID_COUNT];

      void waitForPixels();

    public:
      rgb_t rgb[RGB_COUNT];
      rgbw_t rgbw[RGBW_COUNT];

      void begin();

      void updateRGB(rgb_t * rgbframe);

      void updateRGB(rgb_t * rgbframe, size_t skip, size_t count);

      void updateRGBW(rgbw_t * rgbwframe);

      void updateRGBW_R(uint8_t * red);

      void updateRGBW_G(uint8_t * green);

      void updateSpins(spin_t * spins);

      void updateRaid(diskstat_t * diskstats);

      void step(uint8_t dt);

      void showRGB();

      void showRGBW();

      void clear();
  };
#endif
