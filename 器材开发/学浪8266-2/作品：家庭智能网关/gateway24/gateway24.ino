#include <Arduino.h>
#include <ArduinoJson.h>

#include <espnow.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

/************************以下为可配置项************************/

//智能配网，无需WI-FI账号密码了
//const char* ssid     = "1024";
//const char* password = "12345678";

//来自和风天气的密钥
const String auth = "a8d29b980fac4b8fa188a8b34943539f";

//你的城市ID，获取方法参考课程
const String cityid = "101070102";

//API网址，无需修改(如果您使用付费API，请将https://devapi. 改成 https://api.)
const String weather_apiuri = "https://devapi.qweather.com/v7";

#define ROOM_TOTAL 4

//温湿度刷新
unsigned long lastSensor = 0;
unsigned long lastWeather = 0;
unsigned long lastTime = 0;
unsigned long lastSmart = 0;

const long interval = 2000;
const long interval_time = 1000;
const long intervalWeather = 1000 * 60 * 5;
const long intervalSmart = 100;

long currentMillis = 0;

//调整配色(可用的颜色在课件中有说明)
const uint16_t main_color = TFT_WHITE; //TFT_DARKGREEN 主要前景色
const uint16_t bg_color = TFT_BLACK;  //主要背景色

const uint16_t weather_color = TFT_SILVER; //天气预报的背景色
const uint16_t weather_text_color = TFT_BLACK; //天气预报的文字色

const uint16_t calendar_color = TFT_BLACK; //左上角日历的背景色
const uint16_t calendar_text_color = TFT_WHITE; //左上角日历的文字色

/************************可配置项结束************************/

typedef struct struct_message {
  float temperature = 99;  //温度
  float humidity = 99;     //湿度
  String clientID = "-"; //来消息的客户端ID
  long message_time = 0;  //上一次接收到数据的时间
};

struct_message sensors[ROOM_TOTAL];

//客户端超时时间（指定时间内没有收到消息，视为离线）
const int client_timeout = 1000 * 5;



#include "hefeng-min-40px.h"
#include "weather_font20.h"
#include "weather_font16.h"
#include "weather_font36.h"

//#include "wifi.h"
#include "ntptime.h"
#include "sht30.h"
#include "weather.h"

#include "smartWifi.h"


int x = 0;
int y = 0;
int h = 0;
int line = 0;
//uint8 last_day = 0;

String mac = "";
IPAddress ip;

/*********************************************************setup函数**********/
void setup() {

  Serial.begin(115200);

  //输出调试信息
  Serial.setDebugOutput(true);

  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
  tft.setTextSize(1);
  tft.println("");

  initSmartWiFi();
  //  initWiFi();

  mac = WiFi.macAddress();
  Serial.println(mac);

  //联网成功后，才可以获取IP哦
  ip = WiFi.localIP();


  // Init ESP-NOW
  tft.println("");
  tft.println("Init ESP-NOW");
  if (esp_now_init() != 0) {
    tft.println("Init ESP-NOW Failed");
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  tft.println("Init ESP-NOW SUCCESS");

  //将该设备设为控制端（ ESP_NOW_ROLE_CONTROLLER 可收可发）如果设为 ESP_NOW_ROLE_SLAVE 则只能收。
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  //接收成功后的回调函数，可以在其中获取接收到的数据
  esp_now_register_recv_cb(OnDataRecv);

  tft.println("");
  tft.println("Init NTP time");
  initNtp();
  tft.println("");
  tft.println("Init SHT30");
  sht30_setup();

  delay(500);
  tft.fillScreen(TFT_BLACK);

  //设置文字xy轴的基准线
  //  tft.setTextDatum(MC_DATUM);
  display_data();
}



/*********************************************************循环函数**********/
void loop() {

  currentMillis = millis();

  //智能配网的监听
  if (lastSmart == 0 || currentMillis - lastSmart >= intervalSmart) {
    lastSmart = currentMillis;
    wifi_loop();
  }


  if (lastTime == 0 || currentMillis - lastTime >= interval_time) {
    lastTime = currentMillis;
    loopNtp();
    datetime(false);  //必须在 calendar前调用
    calendar(false);
  }

  //温湿度的刷新
  if (currentMillis - lastSensor > interval) {
    lastSensor = currentMillis;
    sensor(false);
    sht30();
    sensors[0].temperature = sht_data.temperature;
    sensors[0].humidity = sht_data.humidity;
    sensors[0].clientID = "网关";
    sensors[0].message_time = currentMillis;

  }

  //天气的刷新频率
  if (currentMillis - lastWeather > intervalWeather) {
    lastWeather = currentMillis;

    //    Serial.print(" currentMillis:");
    //    Serial.print(currentMillis);
    //    Serial.print(" lastWeather:");
    //    Serial.print(lastWeather);
    //    Serial.print(" intervalWeather:");
    //    Serial.print(intervalWeather);
    //    Serial.println("");
    weather_func(false);
  }

}

/*********************************************************温湿度传感器函数**********/
void sensor(bool init) {
  x = 0;
  y = 190;
  h = 100;
  /**
     只有初始化的时候，刷新整个背景，后续则只刷新内容区域
  */
  if (init) {
    tft.fillRect(x, y, 240 - x, h, bg_color);
  }

  tft.loadFont(weather_font16);
  tft.setTextColor(main_color, bg_color, true);

  //边缘留白
  x = x + 5;
  y = y + 10;
  line = 22;

  tft.drawString("ID=>", x, y);
  tft.drawString("温度", x, y + line);
  tft.drawString("湿度", x, y + line * 2);
  tft.drawString("----", x, y + line * 3);

  for (int i = 0; i < ROOM_TOTAL; i++) {

    x = x + 50;

    //该客户端已经离线，重置其状态
    if ( (currentMillis - sensors[i].message_time) > client_timeout ) {
      sensors[i].clientID = "-";
    }

    if ( sensors[i].clientID != "-" ) {
      tft.drawString(sensors[i].clientID + "  ", x, y);
      tft.drawString(String((int)round(sensors[i].temperature)) + "℃  ", x, y + line, 1);
      tft.drawString(String((int)round(sensors[i].humidity)) + "%  ", x, y + line * 2, 1);
      tft.drawString(" -/- ", x, y + line * 3, 1);
    } else {
      tft.drawString(" -/- ", x, y);
      tft.drawString(" -/- ", x, y + line, 1);
      tft.drawString(" -/- ", x, y + line * 2, 1);
      tft.drawString(" -/- ", x, y + line * 3, 1);
    }
  }
}

/*********************************************************日历函数**********/
void calendar(bool init) {

  x = 0;
  y = 0;

  if (init) {
    tft.fillRect(x, y, 70, 70, calendar_color);
    return;
  }

  //每天刷新一次日历
  //  if ( last_day != 0 && last_day == dt.day ) {
  //    return;
  //  }
  //  Serial.println("init calendar");
  //  last_day = dt.day;

  tft.setTextColor(calendar_text_color, calendar_color, true);
  tft.loadFont(weather_font16);
  tft.drawString("星", x + 10, y + 10);
  tft.drawString("期", x + 10, y + 30);

  String lunarDate = outputLunarDate(dt.year, dt.month, dt.day);

  //闰月用黄色表示
  if (dt.leap) {
    tft.setTextColor(calendar_text_color, calendar_color, true);
  }

  tft.drawCentreString(lunarDate, x + 35, y + 50, 0);
  tft.setTextColor(calendar_text_color, calendar_color, true);

  tft.loadFont(weather_font36);
  tft.drawString(weekOfDate1(dt.year, dt.month, dt.day), x + 30, y + 15);
}

/*********************************************************时间函数**********/
void datetime(bool init) {
  x = 70;
  y = 0;

  if (init) {
    tft.fillRect(x, y, 240 - x, 70, bg_color);
    return;
  }

  tft.loadFont(weather_time);
  tft.setTextColor(main_color, bg_color, true);
  tft.drawString(dt.h + ":" + dt.i, x + 5, y + 5);

  tft.loadFont(weather_font16);
  tft.drawRightString(" " + dt.s + " ", 235, y + 35, 1);

  //每天刷新一次时间(last_day在calendar中更新，这里就不再更新了，必须保证该datetime在 calender前调用)
  //  if ( last_day != 0 && last_day == dt.day ) {
  //    return;
  //  }
  tft.drawRightString(dt.localDate, 235, y + 52, 1);
}

/*********************************************************三日天气函数**********/
void weather_func(bool init) {
  x = 0;
  y = 70;
  line = 22;
  
  if (init) {
    tft.fillRect(x, y, 240, 110, weather_color);
  }
  
  getWeather(); //今日天气
  getTmrWeather();  //最近3天的天气

  for (int i = 0; i < 3; i++) {
    tft.loadFont(hefeng40);
    tft.setTextColor(weather_text_color, weather_color, true);
    if (i == 0) {
      tft.drawCentreString(icon(wd.now_icon), x + 33, y + 5, 0);
    } else {
      tft.drawCentreString(icon(wtd[i].iconDay), x + 33, y + 5, 0);
    }
    tft.unloadFont();

    tft.loadFont(weather_font16);
    tft.setTextColor(weather_text_color, weather_color, true);

    tft.fillRect(x, y + 50, 80, line * 3, weather_color);

    if (i == 0) {
      tft.drawCentreString(wd.now_text, x + 35, y + 55, 0);
      tft.drawCentreString(wd.now_temp + "℃", x + 35, y + 55 + line, 0);
      tft.drawCentreString(wd.now_windScale + "级风" + getWindIcon(wd.now_wind360) , x + 35, y + 55 + line * 2, 0);
    } else {
      tft.drawCentreString(wtd[i].textDay, x + 35, y + 55, 0);
      tft.drawCentreString( wtd[i].tempMin + "℃/" + wtd[i].tempMax + "℃" , x + 35, y + 55 + line, 0);
      tft.drawCentreString( wtd[i].windScaleDay + "级风" + getWindIcon(wtd[i].wind360Day), x + 35, y + 55 + line * 2, 0);
    }


    x += 80;
  }
}

/*****************MAC地址***********************/
void macIP() {
  x = 0;
  y = 300;
  tft.unloadFont();
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
  tft.drawRightString(mac, 239, y, 1);
  tft.drawString(String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]), x, y);
}

//总的调用函数
void display_data() {
  calendar(true);
  datetime(true);
  weather_func(true);
  macIP();
  sensor(true);
}

/**
   客户端发过来的数据
*/
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  struct_message myData;
  memcpy(&myData, incomingData, sizeof(myData));

  Serial.print("Bytes received: ");
  Serial.println(myData.clientID);

  for (int i = 0; i < ROOM_TOTAL; i++) {
    if (sensors[i].clientID == myData.clientID || sensors[i].clientID == "-") {
      sensors[i].temperature = myData.temperature;
      sensors[i].humidity = myData.humidity;
      sensors[i].clientID = myData.clientID;
      sensors[i].message_time = currentMillis; //记录收到消息的时间
      break;
    }
  }
  //    Serial.println(len);
  //    Serial.print("client:");
  //    Serial.print(myData.clientID);
  //    Serial.print(" T: ");
  //    Serial.println(myData.temperature);
  //    Serial.print("H: ");
  //    Serial.println(myData.humidity);
  //    Serial.println();
}
