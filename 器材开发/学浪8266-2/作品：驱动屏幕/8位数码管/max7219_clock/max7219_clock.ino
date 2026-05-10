//连接wifi用的库
#include <ESP8266WiFi.h>

/**
   你的wifi 账号和密码
   注意：切勿连接 5G的频率、不要使用双频融合（路由器）
   注意：账号密码必须完全正确，包括字母大小写、空格、中划线、下划线
*/
const char* ssid     = "1024";
const char* password = "12345678";

#include "LedControl.h"
#include "wifi.h"
#include "ntptime.h"

LedControl lc = LedControl(D8, D7, D6, 1);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("");

  lc.shutdown(0, false);
  lc.setIntensity(0, 15);
  lc.clearDisplay(0);

  //连接WI-FI
  initWiFi();

  //请求网络时间
  initNtp();

  //闪烁显示888888（3次）
  for (int n = 0; n < 3; n++) {
    for (int i = 0; i < 8; i++) {
      lc.setDigit(0, i, 8, false);
    }
    delay(200);
    lc.clearDisplay(0);
    delay(200);
  }
}


unsigned long last_ntp = 0;
const long interval_ntp = 1000; //时间的刷新频率（毫秒）

void loop()
{
  //获取单片机启动至今的毫秒数
  unsigned long currentMillis = millis();

  //update ntp 时间
  if (last_ntp == 0 || currentMillis - last_ntp >= interval_ntp) {

    last_ntp = currentMillis;
    loopNtp();

    //显示（需要分别显示十位、个位，计算的原理就请大家补补数学啦）
    lc.setDigit(0, 7, dt.hours / 10 % 10, false);
    lc.setDigit(0, 6, dt.hours / 1 % 10, false);
    lc.setDigit(0, 4, dt.minutes / 10 % 10, false);
    lc.setDigit(0, 3, dt.minutes / 1 % 10, false);
    lc.setDigit(0, 1, dt.seconds / 10 % 10, false);
    lc.setDigit(0, 0, dt.seconds / 1 % 10, false);
  }
}
