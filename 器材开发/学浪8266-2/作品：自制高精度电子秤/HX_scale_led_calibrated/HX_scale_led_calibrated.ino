#include "HX711.h"
HX711 scale;

//接重量传感器
uint8_t dataPin  = D6;  //DT引脚
uint8_t clockPin = D5;  //SCK引脚

float weight = 0;

#include "screen.h"

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  //做一些初始化工作
  scale.begin(dataPin, clockPin);

  //初始化屏幕
  initScreen();

  //去皮
  scale.tare();
  
  //校准
  scale.set_scale(-475.69);
}


void loop()
{
  //每隔一段时间，输出得到的重量，默认是0.25秒
  weight = scale.get_units(10);
  
  Serial.print("UNITS: ");
  Serial.println(weight);

  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  u8g2.clearBuffer();
  echoScreen(0,30,String(weight,2)+" 克");
  delay(250);
}


// -- END OF FILE --
