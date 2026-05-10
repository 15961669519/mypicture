//包含读取传感器数据的代码，引脚、地址等配置均在该文件
#include "sht30.h"

void setup() {
  Serial.begin(115200);
  
  //sht30的开机配置函数
  sht30_setup();
}

void loop() {
  //sht30的循环读取函数，内设读取间隔可自行在 sht30.h 文件中设置
  sht30();
}
