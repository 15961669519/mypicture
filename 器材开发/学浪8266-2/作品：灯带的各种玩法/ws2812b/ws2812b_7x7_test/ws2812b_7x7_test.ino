#include "ws2812b.h"


void setup() { 
  ws2812b_setup();
  FastLED.setBrightness(32);
}


//自循环函数
void loop() {
  for(int i=0;i<WS_NUM_LEDS;i++){
    ws_leds[i] = CRGB::Red;
    FastLED.show();
    FastLED.delay(100);    
  }
  for(int i=WS_NUM_LEDS-1;i>=0;i--){
    ws_leds[i] = CRGB::Black;
    FastLED.show();
    FastLED.delay(100);    
  }
}
