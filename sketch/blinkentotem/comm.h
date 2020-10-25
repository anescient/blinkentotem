
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
    SPINS // data for spin params
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

  struct spin_params_t {
    uint8_t frequency;
    uint8_t b_min;
    uint8_t b_max;
  };
  #define SPIN_SIZE (RGBW_COUNT * sizeof(spin_params_t))

  class Comm {
    private:
      uint8_t buffer[COMMS_BUFFER_SIZE];

    public:

      void begin();

      datatype_t getData(int timeoutms);

      void exportrgb(rgb_t * dest);

      void exportrgbw(rgbw_t * dest);

      spin_params_t * getSpinParams();
  };
#endif
