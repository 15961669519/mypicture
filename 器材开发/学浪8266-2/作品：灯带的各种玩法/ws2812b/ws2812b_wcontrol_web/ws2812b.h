#include <FastLED.h>

/********************可配置项********************/

//定义灯珠数量
#define WS_NUM_LEDS 49

//定义信号引脚
#define WS_DATA_PIN D4

//定义灯珠的顺序，多数ws2812b 都是GRB的顺序（如果发现颜色显示错误，可以重新排列）
#define WS_COLOR GRB

/********************无需修改********************/

//创建LED管理对象
CRGB ws_leds[WS_NUM_LEDS];

//修改颜色的函数
void ws2812b_color(String colorString){
  
  colorString = colorString.substring(1);
  long rgb = strtol(colorString.c_str(), NULL, 16); 
  
  for(int i=0;i<WS_NUM_LEDS;i++){
    ws_leds[i] = rgb;
    FastLED.show(); 
  }
}

//修改亮度的函数
void ws2812b_brightness(uint16_t value){
  FastLED.setBrightness(value);
  ws2812b_color(led_data.color);
}

//开机管理函数
void ws2812b_setup(){
  //设置芯片型号，和信号引脚，以及RGB灯珠的顺序（GRB 表示绿红蓝）
  FastLED.addLeds<WS2812B, WS_DATA_PIN, WS_COLOR>(ws_leds, WS_NUM_LEDS);
  ws2812b_brightness(led_data.brightness);  
  ws2812b_color(led_data.color);
}
