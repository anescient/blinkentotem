
#include <Adafruit_NeoPixel.h>

#define RGBW_PIN    12
#define RGBW_COUNT  8
#define RGB_PIN     11
#define RGB_COUNT   7

Adafruit_NeoPixel rgbwpix;
Adafruit_NeoPixel rgbpix;

void setup() {
  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();
  
  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();

  Serial.begin(115200);
}

void loop() {
  int r, g, b, w;

  if(Serial.find("xx")) {

    while(Serial.available() < 4 * RGBW_COUNT + 3 * RGB_COUNT);
    
    for(int i = 0; i < RGBW_COUNT; i++) {
      r = Serial.read();
      g = Serial.read();
      b = Serial.read();
      w = Serial.read();
      rgbwpix.setPixelColor(i, g, r, b, w);
    }    
    rgbwpix.show();

    for(int i = 0; i < RGB_COUNT; i++) {
      r = Serial.read();
      g = Serial.read();
      b = Serial.read();
      rgbpix.setPixelColor(i, r, g, b);
    }
    rgbpix.show();
  }
}
