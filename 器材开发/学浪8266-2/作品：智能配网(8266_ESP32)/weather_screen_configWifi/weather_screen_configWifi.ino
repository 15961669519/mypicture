/**
 * 实时天气预报和北京时间
 * by 1024编程实验室 王亮 2022.3.28
 */
 
#include <Arduino.h>

//启用屏幕所需相关库文件
#include <U8g2lib.h>
#include <Wire.h>

//处理json数据
#include <ArduinoJson.h>

//#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

//从ntp服务器更新时间(依赖 ntp 库)
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com");

/******************以下为程序可配置项*******************************/

/**
 * 你的wifi 账号和密码
 * 注意：切勿连接 5G的频率、不要使用双频融合（路由器）
 * 注意：账号密码必须完全正确，包括字母大小写、空格、中划线、下划线
 */
//const char* ssid     = "1024";
//const char* password = "12345678";

//来自和风天气的密钥
const String auth = "7def06ae4c9040d7b4094d2edc5d6da3";

//你的城市ID，获取方法参考课程
const String cityid = "101070102";

//屏幕使用的针脚
const int pin_scl = D2;
const int pin_sda = D1;

/**
 * 设置刷新频率，由于我们需要给时间、天气设置不同的刷新频率
 * 因此不能使用 delay() 而改用这种更灵活专业的方法
 * 官方有例子：示例 - digital - BlinkWithoutDelay （以后会考虑给大家做代码讲解）
 */
unsigned long last_ntp = 0;
const long interval_ntp = 1000; //每秒钟刷新一次屏幕时间（不建议修改，低于一秒没有意义）

unsigned long last_weather = 0;
const long interval_weather = 1000*180; //每3分钟刷新一次天气信息（刷新太快没有意义，因为接口数据更新也有延迟）


/******************程序可配置项【完毕】*******************************/

//数据结构，保存了天气和时间数据
struct weather_data {

  String localDate = "";
  String localTime = "";

  // API状态码，具体含义请参考状态码
  int code = -1;

  //当前API的最近更新时间
  String updateTime = "";

  //数据观测时间
  String now_obsTime = "";

  //温度，默认单位：摄氏度
  int now_temp = 0;

  //体感温度，默认单位：摄氏度
  int now_feelsLike = 0;

  //天气状况和图标的代码，图标可通过天气状况和图标下载
  String now_icon = "";

  //天气状况的文字描述，包括阴晴雨雪等天气状态的描述
  String now_text = "";

  //风向360角度
  int now_wind360 = -1;

  //风向
  String now_windDir = "";

  //风力等级
  int now_windScale = -1;

  //风速，公里/小时
  int now_windSpeed = -1;

  //相对湿度，百分比数值
  int now_humidity = -1;

  //当前小时累计降水量，默认单位：毫米
  int now_precip = -1;

  //大气压强，默认单位：百帕
  int now_pressure = -1;

  //能见度，默认单位：公里
  int now_vis = -1;
} wd;

//根据屏幕的类型，配置并创建对象
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ pin_scl, /* data=*/ pin_sda, /* reset=*/ U8X8_PIN_NONE);

//将screen.ino 改为 .h 方便调用
#include "screen.h"

//连接wifi用的库
#include "wifi.h"

void setup() {
  Serial.begin(115200);

  //初始化屏幕
  initScreen();
  echoScreen(0, 32, "系统启动中...");
  //部分电脑在单片机启动时，串口容易故障，因此做一个等待和刷新的动作
  //它会影响系统启动时间（至少延长4秒）可以删除
  Serial.println();
  Serial.println();
  Serial.println();
  for (uint8_t t = 3; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  //连接wifi
  initWiFi();

  //请求ntp时间
  initNtp();

  //请求天气数据
  getWeather();
}
 
void loop() {
  wifi_loop();
  
  //获取单片机启动至今的毫秒数
  unsigned long currentMillis = millis();

  //更新时间
  if (currentMillis - last_ntp >= interval_ntp) {
    last_ntp = currentMillis;
    loopNtp();
    echoWeather();
  }
  
  //更新天气
  if (currentMillis - last_weather >= interval_weather) {
    last_weather = currentMillis;
    getWeather();
    echoWeather();
  }  
}
