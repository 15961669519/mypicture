#include <FastLED.h>

//定义灯珠数量
#define WS_NUM_LEDS 1

//定义信号引脚
#define WS_DATA_PIN D5

//创建LED管理对象
CRGB ws_leds[WS_NUM_LEDS];

void ws2812b_setup(){
  //设置芯片型号，和信号引脚，以及RGB灯珠的顺序（GRB 表示绿红蓝）
  FastLED.addLeds<WS2812B, WS_DATA_PIN, GRB>(ws_leds, WS_NUM_LEDS);  
}
