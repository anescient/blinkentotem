
#include <Adafruit_NeoPixel.h>

#define RGBW_PIN    12
#define RGBW_COUNT  8
#define RGB_PIN     11
#define RGB_COUNT   8


Adafruit_NeoPixel rgbwpix;
Adafruit_NeoPixel rgbpix;

#define FRAMESIZE (RGBW_COUNT * 4 + RGB_COUNT * 3)
char frame[FRAMESIZE];

void offline() {
  for(int i = 0; i < RGBW_COUNT; i++)
    rgbwpix.setPixelColor(i, 0, 0, 0, 0);
  rgbwpix.show();

  for(int i = 0; i < RGB_COUNT; i++)
    rgbpix.setPixelColor(i, 0, 0, 0);
  rgbpix.setPixelColor(0, 20, 0, 0);
  rgbpix.setPixelColor(1, 20, 0, 0);
  rgbpix.show();
}

void pushframe() {
  size_t frame_i = 0;

  for(int i = 0; i < RGBW_COUNT; i++) {
    int r = frame[frame_i++];
    int g = frame[frame_i++];
    int b = frame[frame_i++];
    int w = frame[frame_i++];
    rgbwpix.setPixelColor(i, g, r, b, w);
  }
  rgbwpix.show();

  for(int i = 0; i < RGB_COUNT; i++) {
    int r = frame[frame_i++];
    int g = frame[frame_i++];
    int b = frame[frame_i++];
    rgbpix.setPixelColor(i, r, g, b);
  }
  rgbpix.show();
}

void setup() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();
  
  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();

  offline();

  Serial.setTimeout(250);
  Serial.begin(115200);
}

void loop() {
  if(Serial.find("xx")) {
    if(Serial.readBytes(frame, FRAMESIZE) == FRAMESIZE) {
      pushframe();
    }
  } else {
    offline();
  }
}
