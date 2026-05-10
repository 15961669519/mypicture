/**
 * 适用于16个灯珠的灯环测试
 * 只要是ws2812b的灯带，够16个灯珠都能正常使用
 */

 #include <FastLED.h>

//定义灯珠数量
#define WS_NUM_LEDS 16

//定义信号引脚
#define WS_DATA_PIN D4

//定义灯珠的顺序，多数ws2812b 都是GRB的顺序（如果发现颜色显示错误，可以重新排列）
#define WS_COLOR GRB

//创建LED管理对象
CRGB ws_leds[WS_NUM_LEDS];

void setup() {
  
  //设置芯片型号，和信号引脚，以及RGB灯珠的顺序（GRB 表示绿红蓝）
  FastLED.addLeds<WS2812B, WS_DATA_PIN, WS_COLOR>(ws_leds, WS_NUM_LEDS);
  //设置灯珠亮度，最高255
  FastLED.setBrightness(32);
}

int n = WS_NUM_LEDS;
int leds_max;
int duration = 20;
int r;
int g;
int b;
int rgb[] = {CRGB::Red,CRGB::Green,CRGB::Blue,CRGB::Yellow};
int step = 0;
void loop() {
  

  //随机出灯珠的顶点（最少3个，最多不能超过灯珠总数的一半，16个灯珠最高8）
  leds_max = random(3, 8);

  //两侧对称，逐一点亮灯珠（涨潮）
  for(int i=0;i<leds_max;i++){
    FastLED.delay(duration<150 ? duration=duration+20 : duration);
    
    ws_leds[i] = rgb[step];
    ws_leds[WS_NUM_LEDS-1-i] = rgb[step];
  }
  
  FastLED.delay(200);

  //让灯珠在亮灭之间闪烁3次
  for(int x=0;x<3;x++){
    
    for(int i=0;i<leds_max;i++){
      ws_leds[i].setRGB(0,0,0);
      ws_leds[WS_NUM_LEDS-1-i].setRGB(0,0,0);
    }
    
    FastLED.delay(200);
    
    for(int i=0;i<leds_max;i++){
      ws_leds[i] = rgb[step];
      ws_leds[WS_NUM_LEDS-1-i] = rgb[step];
    }
    FastLED.delay(200);
  }

  //反向逐一灭掉灯珠(退潮)
  for(int i=leds_max-1;i>=0;i--){
    FastLED.delay(50);
    ws_leds[i].setRGB(0,0,0);
    ws_leds[WS_NUM_LEDS-1-i].setRGB(0,0,0);
  }
  step++;
  step = step>3 ? 0 : step;
}
