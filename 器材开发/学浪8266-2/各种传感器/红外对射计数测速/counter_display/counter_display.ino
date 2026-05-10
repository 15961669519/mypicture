/**
   1024编程实验室，王亮编写于 2023年2月11日
   官方正版抖音号：1024devel（1024编程实验室）、iot1024（创联智能）
   有任何问题，请在学浪班级群给我留言。
*/
#include <Arduino.h>

//数字传感器的引脚
#define SENSOR D5

//计数初始化值
long i = 0;

//每隔一定时间刷新一次屏幕（屏幕刷新时可能会导致计数不准）
unsigned long previousMillis = 0;
const long interval = 100;  //100表示0.1秒刷新一次屏幕
unsigned long currentMillis = 0;

//初始化屏幕驱动
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//根据数字长度计算屏幕应该显示的坐标值
int getNumberWidth(long i, int w) {
  if (i < 10) {
    return 1 * w / 2;
  }
  if (i < 100) {
    return 2 * w / 2;
  }
  if (i < 1000) {
    return 3 * w / 2;
  }
  if (i < 10000) {
    return 4 * w / 2;
  }
  if (i < 100000) {
    return 5 * w / 2;
  }
  if (i < 1000000) {
    return 6 * w / 2;
  }
  if (i < 10000000) {
    return 7 * w / 2;
  }
}

//中断函数，保证计数准确
ICACHE_RAM_ATTR void counter() {
  i++;
}

void setup() {
  Serial.begin(115200);

  Serial.println("...");

  pinMode(SENSOR, INPUT_PULLUP);
  
  //每次从低电平向高电平切换时，就触发中断
  attachInterrupt(digitalPinToInterrupt(SENSOR), counter, RISING);

  //屏幕模块的准备工作
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  
  //显示标题
  u8g2.setCursor(22, 20);
  u8g2.print("红外计数器");
  u8g2.sendBuffer();
  
  //设置数字字体
  u8g2.setFont(u8g2_font_logisoso24_tn);
}


void loop() {
  
  //刷新屏幕
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    //定义数字的显示区域
    int x = 0;
    int y = 32;
    int f = 24;

    //清除屏幕指定区域
    u8g2.setDrawColor(0);
    u8g2.drawBox(x, y, 128, f);
    
    //显示当前计数器的值  
    u8g2.setCursor(64 - getNumberWidth(i, 16), y + f);
    u8g2.setDrawColor(1);
    u8g2.print(i);
    u8g2.updateDisplayArea(x, y / 8, 128 / 8, f / 8);
  }
}
