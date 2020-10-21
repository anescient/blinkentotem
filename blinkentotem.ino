
#include <Adafruit_NeoPixel.h>

#define RGBW_PIN    12
#define RGBW_COUNT  8
#define RGB_PIN     11
#define RGB_COUNT   8

struct rgb_t {
  char r;
  char g;
  char b;
};
rgb_t rgb[RGB_COUNT];
#define RGB_SIZE (RGB_COUNT * sizeof(rgb_t))

struct rgbw_t {
  char r;
  char g;
  char b;
  char w;
};
rgbw_t rgbw[RGBW_COUNT];
#define RGBW_SIZE (RGBW_COUNT * sizeof(rgbw_t))

#define HEADER_MAXSIZE  4
char header[HEADER_MAXSIZE];

Adafruit_NeoPixel rgbpix;
Adafruit_NeoPixel rgbwpix;

unsigned int idlecycles;

void showRGB() {
  for(int i = 0; i < RGB_COUNT; i++) {
    rgb_t & c = rgb[i];
    rgbpix.setPixelColor(i, c.r, c.g, c.b);
  }
  digitalWrite(13, HIGH);
  rgbpix.show();
  digitalWrite(13, LOW);
}

void showRGBW() {
  for(int i = 0; i < RGBW_COUNT; i++) {
    rgbw_t & c = rgbw[i];
    rgbwpix.setPixelColor(i, c.g, c.r, c.b, c.w);
  }
  digitalWrite(13, HIGH);
  rgbwpix.show();
  digitalWrite(13, LOW);
}

void clear() {
  memset(rgb, 0, RGB_SIZE);
  memset(rgbw, 0, RGBW_SIZE);
}

void offline() {
  clear();
  showRGBW();
  rgb[0].r = 20;
  rgb[1].r = 20;
  showRGB();
}

void setup() {
  idlecycles = 0;

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(RGBW_PIN, OUTPUT);
  rgbwpix = Adafruit_NeoPixel(RGBW_COUNT, RGBW_PIN, NEO_RGBW);
  rgbwpix.begin();

  pinMode(RGB_PIN, OUTPUT);
  rgbpix = Adafruit_NeoPixel(RGB_COUNT, RGB_PIN);
  rgbpix.begin();

  offline();

  Serial.setTimeout(10);
  Serial.begin(115200);
}

void loop() {
  if(idlecycles > 100) {
    offline();
    idlecycles = 0;
    return;
  }

  if(Serial.find('$')) {
    int headersize = Serial.readBytesUntil('\0', header, HEADER_MAXSIZE);
    if(headersize == 0)
      return;

    switch(header[0]) {
      case '1':
        if(Serial.readBytes((char*)rgb, RGB_SIZE) == RGB_SIZE)
          showRGB();
        break;

      case '2':
        if(Serial.readBytes((char*)rgbw, RGBW_SIZE) == RGBW_SIZE)
          showRGBW();
        break;

      default:
        break;
    }

    idlecycles = 0;
  } else {
    idlecycles++;
  }
}
