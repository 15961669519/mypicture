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

  //输出读取到的数据（克）
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  

  //在串口监视器输入任意字符，开始去皮
  Serial.println("\nEmpty the scale, press a key to continue");
  Serial.println("\n输入任意字符，开始去皮");
  
  
  echoScreen(0,20,"按任意键开始去皮");
  
  while(!Serial.available());
  while(Serial.available()){
    Serial.print(Serial.read());  
  }

  //去皮
  scale.tare();
  weight = scale.get_units(10);
  Serial.print("UNITS: ");
  Serial.println(weight);

  //放一个已知重量的砝码，之后在串口输入任意内容，开始校准
  
  Serial.println("\nPut 506 gram in the scale, press a key to continue");
  Serial.println("\n放一个506克砝码，之后输入任意字符校准");
  echoScreen(0,40,"放506克砝码校准");
  
  while(!Serial.available());
  
  while(Serial.available()){
    Serial.print(Serial.read());  
  }
  
  //你有多大的砝码，就写多少，这里是506克
  scale.calibrate_scale(506, 5);
  
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  //校准后，在串口输入任意内容，开始承重。
  Serial.println("\nScale is calibrated, press a key to continue");
  Serial.println("\n输入任意字符开始称重");
  echoScreen(0,60,"按任意键开始称重");
  
  //显示校准的数值，将该数值通过set_scale 写入，就不用开机重新校准了。
  Serial.println(scale.get_scale());
//  Serial.println(scale.get_offset());
  
  //scale.set_scale(111.11);
  
  while(!Serial.available());
  while(Serial.available()){
    Serial.print(Serial.read());  
  }

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
