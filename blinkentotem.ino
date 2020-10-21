
#include <Adafruit_NeoPixel.h>

#define RGBW_PIN    12
#define RGBW_COUNT  8
#define RGB_PIN     11
#define RGB_COUNT   8

Adafruit_NeoPixel rgbwpix;
Adafruit_NeoPixel rgbpix;

#define RGB_SIZE    (RGB_COUNT * 3)
#define RGBW_SIZE   (RGBW_COUNT * 4)
char buf[RGB_SIZE + RGBW_SIZE];

void showRGB() {
  digitalWrite(13, HIGH);
  rgbpix.show();
  digitalWrite(13, LOW);
}

void showRGBW() {
  digitalWrite(13, HIGH);
  rgbwpix.show();
  digitalWrite(13, LOW);
}

void offline() {
  rgbwpix.clear();
  showRGBW();

  rgbpix.clear();
  rgbpix.setPixelColor(0, 20, 0, 0);
  rgbpix.setPixelColor(1, 20, 0, 0);
  showRGB();
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();
  
  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();

  offline();

  Serial.setTimeout(750);
  Serial.begin(115200);
}

void loop() {
  if(Serial.find("np")) {
    if(Serial.readBytes(buf, 1) != 1) { return; }
    bool do_rgb;
    if(buf[0] == '1') {
      do_rgb = false;
    } else if(buf[0] == '2') {
      do_rgb = true;
    } else {
      return;
    }

    if(Serial.readBytes(buf, RGBW_SIZE) != RGBW_SIZE) { return; }
    if(do_rgb && Serial.readBytes(buf + RGBW_SIZE, RGB_SIZE) != RGB_SIZE) { return; }

    int buf_i = 0;
    for(int i = 0; i < RGBW_COUNT; i++) {
      int r = buf[buf_i++];
      int g = buf[buf_i++];
      int b = buf[buf_i++];
      int w = buf[buf_i++];
      rgbwpix.setPixelColor(i, g, r, b, w);
    }
    showRGBW();

    if(do_rgb) {
      for(int i = 0; i < RGB_COUNT; i++) {
        int r = buf[buf_i++];
        int g = buf[buf_i++];
        int b = buf[buf_i++];
        rgbpix.setPixelColor(i, r, g, b);
      }
      showRGB();
    }

  } else {
    offline();
  }
}
