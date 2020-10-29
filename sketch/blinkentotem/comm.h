
#ifndef COMM_H
  #define COMM_H

  #include <Arduino.h>
  #include <HardwareSerial.h>
  #include "hwconfig.h"

  // rgbw is the largest
  #define COMMS_BUFFER_SIZE RGBW_SIZE

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

  struct rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
  #define RGB_SIZE (RGB_COUNT * sizeof(rgb_t))

  struct rgbw_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
  };
  #define RGBW_SIZE (RGBW_COUNT * sizeof(rgbw_t))

  struct spin_t {
    uint8_t frequency;
    uint8_t brightness;
  };

  class Comm {
    private:
      uint8_t buffer[COMMS_BUFFER_SIZE];

    public:

      void begin();

      datatype_t getData(int timeoutms);

      spin_t * getSpins();

      void exportrgb(rgb_t * dest);

      void exportrgb(rgb_t * dest, size_t index);

      void exportrgbw(rgbw_t * dest);

      void exportrgbw(rgbw_t * dest, size_t index);

      void exportrgbw_red(rgbw_t * dest);

      void exportrgbw_green(rgbw_t * dest);
  };
#endif
