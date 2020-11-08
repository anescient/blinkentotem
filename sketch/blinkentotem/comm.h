
#ifndef COMM_H
  #define COMM_H

  #include <Arduino.h>
  #include <HardwareSerial.h>
  #include "hwconfig.h"

  #define COMM_BUFFER_SIZE (RGBW_COUNT * sizeof(rgbw_t))

  enum datatype_t {
    NONE, // timeout or error or whatever
    PING, // watchdog reset
    FLUSH, // update led strings immediately
    CLEAR, // clear all colors and effects
    CONFIG,
    RGBFRAME, // entire rgb string
    RGBWFRAME, // entire rgbw string
    CPU_SPIN_B, // spins on rgbw blue
    CPU_FADE_G, // faders on rgbw green
    CPU_FADE_W, // faders on rgbw white
    CPU_GLOW_R, // uint8_t[8]
    LAMP, // rgb[0] to rgb[1]
    DRUM, // rgb[2] to rgb[3]
    RAID, // rgb[4] to rgb[7]
    RAIDFLASH, // iopulse_t[4]
    DRUMFLASH // iopulse_t[2]
  };

  struct rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  struct rgbw_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
  };

  struct config_t {
    uint8_t raidRed;
    uint8_t raidGreen;
    uint8_t drumRed;
    uint8_t drumGreen;
  };

  struct spin_t {
    uint8_t frequency;
    uint8_t brightness;
  };

  struct fade_t {
    uint8_t value;
    uint8_t decayrate;
  };

  struct iopulse_t {
    uint8_t read;
    uint8_t write;
  };

  class Comm {
    public:

      union {
        uint8_t     bytes [COMM_BUFFER_SIZE];
        char        command;
        config_t    config;
        rgb_t       rgb [RGB_COUNT];
        rgbw_t      rgbw [RGBW_COUNT];
        spin_t      spins [RGBW_COUNT];
        fade_t      fades [RGBW_COUNT];
        iopulse_t   pulses [RAID_COUNT];
      } buffer;

      void begin();

      datatype_t receive(int timeoutms);
  };
#endif
