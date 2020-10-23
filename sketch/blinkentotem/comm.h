
#ifndef COMM_H
  #define COMM_H

  #include "hwconfig.h"
  #include <HardwareSerial.h>

  #define COMMS_BUFFER_SIZE RGBW_SIZE

  enum datatype_t {
    NONE, // or error or whatever
    RGBFRAME, // data for entire rgb string
    RGBWFRAME, // data for entire rgbw string
    SPINS
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

  class Comm {
    private:
      char buffer[COMMS_BUFFER_SIZE];

    public:

      void begin();

      datatype_t getData(int timeoutms);

      void exportrgb(rgb_t * dest);

      void exportrgbw(rgbw_t * dest);
  };
#endif
