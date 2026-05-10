/**
 * 1024编程实验室，王亮编写于 2023年2月11日
 * 官方正版抖音号：1024devel（1024编程实验室）、iot1024（创联智能）
 * 有任何问题，请在学浪班级群给我留言。
 * 
 * 可以输出通用传感器的数字、模拟信号值，方便我们做测试
 */
#include <Arduino.h>

//包含字库
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println();
  Serial.println();

  //屏幕模块的准备工作
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
}

//用于记录模拟量、上一次采集到的模拟量的值
int a0 = 0;
int a0_last = 0;

void loop() {
  //清除屏幕内容
  u8g2.clearBuffer();

  //显示标题
  u8g2.setCursor(0, 20);
  u8g2.print("传感器测试仪");
  
  //采集数字量信号
  u8g2.setCursor(0, 40);
  u8g2.print("D5 数字量: ");
  u8g2.print(digitalRead(D5));

  //采集模拟量信号
  u8g2.setCursor(0, 60);
  u8g2.print("A0 模拟量: ");
  a0 = analogRead(A0);
  //如果模拟量信号波动在正负5之间，则不刷新数据（模拟量的正常波动）
  if(a0<a0_last+5 && a0>a0_last-5){
    a0 = a0_last;  
  }else{
    a0_last = a0;  
  }
  u8g2.print(a0);
  u8g2.sendBuffer();
}
