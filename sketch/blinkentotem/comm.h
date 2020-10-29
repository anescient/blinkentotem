
#ifndef COMM_H
  #define COMM_H

  #include <Arduino.h>
  #include <HardwareSerial.h>
  #include "hwconfig.h"

  #define COMMS_BUFFER_SIZE (RGBW_COUNT * sizeof(rgbw_t))

  enum datatype_t {
    NONE, // timeout or error or whatever
    PING, // watchdog reset
    RGBFRAME, // data for entire rgb string
    RGBWFRAME, // data for entire rgbw string
    CPU_BLUE, // data for spins on rgbw blue
    CPU_RED, // 8 bytes, rgbw string red
    CPU_GREEN, // 8 bytes, rgbw string green
    LAMPS, // rgb[0] to rgb[1]
    DRUM, // rgb[2] to rgb[3]
    RAID // rgb[4] to rgb[7]
  };

  // used for both comm and framebuffer
  struct rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  // used for both comm and framebuffer
  struct rgbw_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
  };

  struct spin_t {
    uint8_t frequency;
    uint8_t brightness;
  };

  class Comm {
    public:

      union {
        uint8_t   bytes[COMMS_BUFFER_SIZE];
        char      command;
        rgb_t     rgb[RGB_COUNT];
        rgbw_t    rgbw[RGBW_COUNT];
        spin_t    spins[RGBW_COUNT];
      } buffer;

      void begin();

      datatype_t receive(int timeoutms);
  };
#endif
