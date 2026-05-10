//启用屏幕所需相关库文件
#include <U8g2lib.h>
#include <Wire.h>

//根据屏幕的类型，配置并创建对象
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D1, /* data=*/ D2, /* reset=*/ U8X8_PIN_NONE);

//初始化屏幕
void initScreen() {
  u8g2.begin();
  //定义为UTF8字符集（否则不支持中文）
  u8g2.enableUTF8Print();
  //定义字体（不能随便改）
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
}

//向屏幕输出内容
void echoScreen(int col, int row,const String text) {
  //定位光标
  u8g2.setCursor(col, row);
  //显示内容
  u8g2.print(text);
  //发送到屏幕
  u8g2.sendBuffer();
}
