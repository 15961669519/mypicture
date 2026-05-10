#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "GxEPD2_display_selection_new_style.h"
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

//根据自己的屏幕驱动芯片、接线，初始化屏幕
GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> display(GxEPD2_290_T94(/*CS=D8 d6*/ D1, /*DC=D3*/ D3, /*RST=D4*/ D4, /*BUSY=D2 d8*/ D2)); // GDEH0154D67

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void setup()
{
  display.init(115200);

  //设置屏幕方向
  display.setRotation(1);

  //加载字库
  u8g2Fonts.begin(display);

  //对屏幕的显示操作
  helloWorld();

  //进入深度睡眠
  display.hibernate();
}

void helloWorld() {

  u8g2Fonts.setFontMode(1);                   // 透明模式（默认如此，并非深究）
  u8g2Fonts.setFontDirection(0);              // 从左到右
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // 文字颜色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 北京颜色
  u8g2Fonts.setFont(u8g2_font_wqy16_t_chinese3);

  //设置要操作的窗口范围（full=整个屏幕）
  display.setFullWindow();

  //分页缓冲，可以避免单片机内存不足（刷新速度会略慢）
  display.firstPage();
  do
  {
    //要执行的相关操作
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(110, 70);
    u8g2Fonts.print("你好中国");
  }
  while (display.nextPage());
}

//空空如也
void loop() {};
