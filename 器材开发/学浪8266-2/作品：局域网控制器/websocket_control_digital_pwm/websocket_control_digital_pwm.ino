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
//#include <map>
struct PINDATA {
  uint8_t dname[8] = {D1, D2, D3, D4, D5, D6, D7, D8};
//  std::map<String,uint8_t> dnameString = {
//    { "d1", 0 },
//    { "d2", 1 },
//    { "d3", 2 },
//    { "d4", 3 },
//    { "d5", 4 },
//    { "d6", 5 },
//    { "d7", 6 },
//    { "d8", 7 }
//};
  uint16_t d[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
}
