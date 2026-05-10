#include "ws2812b.h"


void setup() { 
  ws2812b_setup();
}


//自循环函数
void loop() {
  ws_leds[0] = CRGB::Red;
  FastLED.show();
  FastLED.delay(1000);
  
  ws_leds[0] = CRGB::Black;
  FastLED.show();
  FastLED.delay(1000);
}
