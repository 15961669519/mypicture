//********sht30 温湿度传感器***************

#include <Wire.h>
#include "ClosedCube_SHT31D.h"
ClosedCube_SHT31D sht3xd;

// 配置引脚
const int SHT_SCL = D1; 
const int SHT_SDA = D2;

const int SHT_ADDRESS = 0x44;

//配置读取频率

struct SHT_DATA{
  int8_t temperature = -99;
  int8_t humidity = -99;
  unsigned long sht30_last = 0;
const long sht30_interval = 5000; //每秒读取一次

} sht_data;

//开机函数
void sht30_setup() {
  
  //定义I2C通讯的引脚
  Wire.begin(SHT_SDA, SHT_SCL);
  
  //准备读取sht3x传感器
  sht3xd.begin(SHT_ADDRESS);
  if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR){
    Serial.println("[ERROR] 读取sht30数据失败，可能是传感器没插好");
  }
}

//保存结果到指定的数据结构，如果传感器断开，则尝试重载
void saveResult(SHT31D result) {
  if (result.error == SHT3XD_NO_ERROR) {
    sht_data.temperature = result.t;
    sht_data.humidity = result.rh;
  } else {  
    sht_data.temperature = -98;
    sht_data.humidity = -98;
    sht30_setup();
  }
}

//循环体函数，放在loop中
void sht30() {
  unsigned long currentMillis = millis();
  if (currentMillis - sht_data.sht30_last >= sht_data.sht30_interval) {
    sht_data.sht30_last = currentMillis;
    saveResult(sht3xd.periodicFetchData());
    Serial.print("T:");
    Serial.print(sht_data.temperature);
    Serial.print(" H:");
    Serial.println(sht_data.humidity);

    //为了尽量降低屏幕刷新区域，把这两个字放到 天气那边去显示了。
//    u8g2.setCursor(0, 60);
//    u8g2.print("温 ");
    u8g2.setCursor(16, 60);
    u8g2.print(String(sht_data.temperature));
//    u8g2.setCursor(32, 60);
//    u8g2.print("湿 ");
    u8g2.setCursor(56, 60);
    u8g2.print(String(sht_data.humidity));

    //刷新温度屏幕区域 x=2*8=16 y=6*8=48,刷新一个 16*16 的区域
    u8g2.updateDisplayArea(2, 6, 2, 2);
    
    //刷新湿度屏幕区域 x=7*8=56 y=6*8=48,刷新一个 16*16 的区域
    u8g2.updateDisplayArea(7, 6, 2, 2);
  }
}
