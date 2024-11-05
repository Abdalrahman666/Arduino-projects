#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 256

#define DATA_PIN 13

//Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(150);
}

void loop() {
  int j=0;
  for (int i=0; i<256; i++) {
    leds[i].setHSV(0+j, 255, 255-j);
    FastLED.show();
    delay(50);
    FastLED.clear();
    FastLED.show();
    j = j+3;
  }
}