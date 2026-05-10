#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>

#include "GxEPD2_display_selection_new_style.h"

//我们的图片文件，就保存在这了
#include "arrow.h"

//根据自己的屏幕驱动芯片、接线，初始化屏幕
GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> display(GxEPD2_290_T94(/*CS=D8*/ D1, /*DC=D3*/ D3, /*RST=D4*/ D4, /*BUSY=D2*/ D2)); // GDEH0154D67

void setup()
{
  display.init(115200);
  
  //设定屏幕方向
  display.setRotation(1);
  
  //设定屏幕背景色
  display.fillScreen(GxEPD_WHITE);
  
  //定义屏幕区域
  display.setFullWindow();
  
  //绘图
  display.drawBitmap(0, 0, a, _width, _height, GxEPD_BLACK);
  
  //显示
  display.display();
  
  //进入深度睡眠
  display.hibernate();
}

void loop() {

};
