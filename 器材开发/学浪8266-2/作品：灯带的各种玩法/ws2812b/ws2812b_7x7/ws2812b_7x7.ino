#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

const char* ssid = "1024";
const char* password = "12345678";

//心跳频率（用于客户端和服务端的通讯保持）
uint16_t heart_timeout = 8000;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


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
