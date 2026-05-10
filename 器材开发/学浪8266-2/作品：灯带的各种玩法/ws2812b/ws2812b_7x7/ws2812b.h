#include <FastLED.h>

/********************可配置项********************/

//定义灯珠数量
#define WS_NUM_LEDS 49

//定义信号引脚
#define WS_DATA_PIN D5

//定义灯珠的顺序，多数ws2812b 都是GRB的顺序（如果发现颜色显示错误，可以重新排列）
#define WS_COLOR GRB

/**
   用一个struct 保存引脚的数据，检测到任何一个引脚发生变化，则将整个struct 发送给客户端
*/
struct LEDDATA {
  String color = "#000000"; //开机后，灯珠的默认颜色
  uint16_t brightness = 64;  //开机后的默认亮度

  String colors[WS_NUM_LEDS];      //每个灯珠的16进制颜色（仅用于和客户端通信）
  long colors_hex[WS_NUM_LEDS];    //每个灯珠计算后的RGB值
} led_data;


/********************无需修改********************/

//创建LED管理对象
CRGB ws_leds[WS_NUM_LEDS];

//修改颜色的函数
void ws2812b_color(){
  for(int i=0;i<WS_NUM_LEDS; i++){
    ws_leds[i] = led_data.colors_hex[i];
    FastLED.show();
  }
}

//修改亮度的函数
void ws2812b_brightness(uint16_t value){
  FastLED.setBrightness(value);
  ws2812b_color();
}

//开机管理函数
void ws2812b_setup(){
  
  //初始化默认颜色
  String colorString = led_data.color.substring(1);
  for(int i=0;i<WS_NUM_LEDS;i++){
    led_data.colors[i] = led_data.color;
    //计算hex的值
    long rgb = strtol(colorString.c_str(), NULL, 16);
    led_data.colors_hex[i] = rgb;
  }
  
  //设置芯片型号，和信号引脚，以及RGB灯珠的顺序（GRB 表示绿红蓝）
  FastLED.addLeds<WS2812B, WS_DATA_PIN, WS_COLOR>(ws_leds, WS_NUM_LEDS);
  ws2812b_brightness(led_data.brightness);  
  ws2812b_color();
}
