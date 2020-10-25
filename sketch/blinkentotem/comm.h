
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
    char r;
    char g;
    char b;
  };
  #define RGB_SIZE (RGB_COUNT * sizeof(rgb_t))

  struct rgbw_t {
    char r;
    char g;
    char b;
    char w;
  };
  #define RGBW_SIZE (RGBW_COUNT * sizeof(rgbw_t))

  struct spin_params_t {
    char frequency;
    char b_min;
    char b_max;
  };
  #define SPIN_SIZE (RGBW_COUNT * sizeof(spin_params_t))

  class Comm {
    private:
      char buffer[COMMS_BUFFER_SIZE];

    public:

      void begin();

      datatype_t getData(int timeoutms);

      void exportrgb(rgb_t * dest);

      void exportrgbw(rgbw_t * dest);

      spin_params_t * getSpinParams();
  };
#endif
