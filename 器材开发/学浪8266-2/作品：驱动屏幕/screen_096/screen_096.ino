#include <Arduino.h>

#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D2, /* data=*/ D1, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println();
  Serial.println();
  
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  
  u8g2.clearBuffer();
  u8g2.setCursor(0, 30);
  u8g2.print("你好，世界2！");

  u8g2.setCursor(0, 60);
  u8g2.print("10224编程实验室！");

  u8g2.sendBuffer();
}

void loop() {
  //如果需要刷新屏幕，可以在这里继续写代码
}
