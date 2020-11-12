
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

    CONFIG, // config_t

    RGBFRAME, // rgb_t[8] entire rgb string
    DRUM, // rgb[2] to rgb[3]
    RAID, // rgb[4] to rgb[7]
    LAMP, // rgb[0] to rgb[1]
    RAIDFLASH, // iopulse_t[4]
    DRUMFLASH, // iopulse_t[2]

    RGBWFRAME, // rgbw_t[8] entire rgbw string
    RGBW_FADE_R, // fade_t[8] for rgbw red
    RGBW_FADE_G, // fade_t[8] for rgbw green
    RGBW_SPIN_B, // spin_t[8] for rgbw blue
    RGBW_FADE_W, // fade_t[8] for rgbw white
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
    uint8_t targetvalue;
    uint8_t uprate;
    uint8_t downrate;
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
