/******************以下为程序可配置项*******************************/

/**
   你的wifi 账号和密码
   注意：切勿连接 5G的频率、不要使用双频融合（路由器）
   注意：账号密码必须完全正确，包括字母大小写、空格、中划线、下划线
*/
const char* ssid     = "1024";
const char* password = "12345678";

//来自和风天气的密钥85f79dbb78a0452d8b544e4a9d6dc9d5
const String auth = "85f79dbb78a0452d8b544e4a9d6dc9d5";

//你的城市ID，获取方法参考课程
const String cityid = "101070102";

//时间刷新频率
unsigned long last_ntp = 0;
const long interval_ntp = 1000 * 1; //时间的刷新频率（毫秒）

//天气的刷新频率
unsigned long last_weather = 0;
const long interval_weather = 1000 * 60 * 10; //时间的刷新频率（毫秒）

//传感器刷新频率
unsigned long last_sensor = 0;
const long interval_sensor = 1000 * 60; //时间的刷新频率（毫秒）

//处理json数据
#include <ArduinoJson.h>

#include "wifi.h"     //联网
#include "ntptime.h"  //时间
#include "weather.h"  //天气
#include "sht30.h"  //温湿度
#include "bmp180.h" //气压海拔
#include "sgp30.h"  //空气质量
#include "q.c"      //图标字体


//和屏幕显示有关的函数
#include "display.h"

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  display_init();
  display_on();

  //连接wifi
  initWiFi();
  //初始化ntp时间服务器
  initNtp();

  getWeather();
  getTmrWeather();

  sht30_setup();
  bmp180_setup();
  sgp30_setup();
}

void loop() {
  //获取单片机启动至今的毫秒数
  unsigned long currentMillis = millis();

  //更新传感器
  if (last_sensor == 0 || currentMillis - last_sensor >= interval_sensor) {
    last_sensor = currentMillis;
    sht30();
    bmp180();
    sgp30();
  }

  //更新时间等数据
  if (last_ntp == 0 || currentMillis - last_ntp >= interval_ntp) {
    last_ntp = currentMillis;
    loopNtp();
    resultDisplay();
  }

  //更新天气
  if (last_weather == 0 || currentMillis - last_weather >= interval_weather) {
    last_weather = currentMillis;
    getWeather();
    getTmrWeather();
  }

}
