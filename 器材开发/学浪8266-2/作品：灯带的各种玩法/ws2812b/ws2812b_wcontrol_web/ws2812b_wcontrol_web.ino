#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

const char* ssid = "1024";
const char* password = "12345678";

//心跳频率，决定了数据的刷新频率
uint16_t heart_timeout = 2000;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

/**
   用一个struct 保存引脚的数据，检测到任何一个引脚发生变化，则将整个struct 发送给客户端
*/
struct LEDDATA {
  String color = "#000000";
  uint16_t brightness = 0;
  uint16_t last_brightness = 0;
} led_data;

#include "ws2812b.h";
#include "html.h";
#include "ws.h";

//开机函数
void setup() {
  
  Serial.begin(115200);

  //连接wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  ws2812b_setup();
  
  //启动websocket
  initWebSocket();
  
  //创建HTML页面和请求
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.begin();

  
}

//自循环函数
void loop() {
  ws.cleanupClients();
}
