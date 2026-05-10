#include <Arduino.h>

#include <U8g2lib.h>
#include <Wire.h>
#include "myfont.h"
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D4, /* data=*/ D3, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);

  //先清空屏幕
  u8g2.clearBuffer();
  
  //绘制文字
  u8g2.drawXBM(0, 0, 32, 32, myfonts[0]);
  u8g2.drawXBM(32, 0, 32, 32, myfonts[1]);
  
  //将绘制的内容，显示在屏幕上
  u8g2.sendBuffer();
}

//定义变量，记录当前的图像帧数
int  frame = 0;

void loop() {
  //绘制图像
  u8g2.drawXBM(64, 0, 64, 64, animation[frame]);

  //刷新图像区域（避免全局刷新闪烁，注意这里的数值要*8 才是真实像素）
  // x=8*8=64,y=0,w=8*8,h=8*8
  u8g2.updateDisplayArea(8, 0, 8, 8);
  
  //帧数+1
  frame++;
  
  //如果帧数超出上限，则归零重新显示
  if (frame >= T) {
    frame = 0;
  }
}
