#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

const char* ssid = "1024";
const char* password = "12345678";

bool ledState = 0;
const int ledPin = LED_BUILTIN;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


/**
   用一个struct 保存引脚的数据，检测到任何一个引脚发生变化，则将整个struct 发送给客户端
*/

struct PINDATA {
  uint8_t dname[8] = {D1, D2, D3, D4, D5, D6, D7, D8};
  uint8_t d[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  //上一次触发时间
  unsigned long dt[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint16_t a0 = 0;
} pin_data;

#include "html.h";
#include "ws.h";

void setup() {
  
  Serial.begin(115200);
  
  //设置默认的电平
  for (uint8_t i=0; i<8; i++) {
    pinMode(pin_data.dname[i], OUTPUT);
  }
  digitalWrite(ledPin, LOW);

  //连接wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());
  
  //启动websocket
  initWebSocket();
  
  //创建HTML页面和请求
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
  unsigned long currenttime = millis();

  //检查引脚的状态
  bool is_changed = false;
  for (int8 i = 0; i < 8; i++) {
    int8_t tmp = digitalRead(pin_data.dname[i]);
    if ( pin_data.d[i] != tmp ) {
      //防止误触发（<50毫秒的信号将忽略）
      if ( currenttime - pin_data.dt[i] > 50 ) {
        pin_data.dt[i] = currenttime;
        is_changed = true;
        pin_data.d[i] = tmp;
      }
    }
  }
  //如果引脚数据发生变化，就发送给所有客户端
  if (is_changed) {
    Serial.println("is changed");
    sendPinDatas();
  }
}
