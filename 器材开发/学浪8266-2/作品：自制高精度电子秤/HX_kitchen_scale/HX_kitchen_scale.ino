#include "HX711.h"

HX711 scale;

//uint8_t dataPin = 6;
//uint8_t clockPin = 7;
uint8_t dataPin  = D6;//for esp8266
uint8_t clockPin = D5;//for esp8266

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  //初始化称重模块
  scale.begin(dataPin, clockPin);

  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  //去皮
  Serial.println("\nEmpty the scale, press a key to continue");
  Serial.println("\n输入任意字符，开始去皮");
  while (true) { //...}
    while (!Serial.available());
    while (Serial.available()) Serial.read();

    scale.tare();
    Serial.print("UNITS: ");
    Serial.println(scale.get_units(10));
    //校准
    Serial.println("\nPut 506 gram in the scale, press a key to continue");
    Serial.println("\n放一个506克砝码，之后输入任意字符校准");
    while (!Serial.available());
    while (Serial.available()) Serial.read();

    scale.calibrate_scale(506, 5);
    Serial.print("UNITS: ");
    Serial.println(scale.get_units(10));

    //称重
    Serial.println("\n输入任意字符开始称重");
    Serial.println(scale.get_scale());
    //  Serial.println(scale.get_offset());
    while (!Serial.available());
    while (Serial.available()) {
      Serial.read();
    }
  }

}
void loop()
{
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  delay(250);
}


// -- END OF FILE --
