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
//#include <map>
struct PINDATA {
  uint8_t dname[4] = {0, 2, 1, 3};
  uint16_t d[4] = {0, 0, 0, 0};
  
  //上一次触发时间
  unsigned long dt[4] = {0, 0, 0, 0};
} pin_data;

#include "html.h";
#include "ws.h";



/**
 * 检查指定引脚状态是否发生变化
 * 可设定检查频率
 */
void check_pin(int8_t index, bool& is_changed, bool is_digital = true, uint16_t interval=50){
  unsigned long currenttime = millis();

  //每隔指定时间，采集一次数据
  if ( currenttime - pin_data.dt[index] > interval ) {
    pin_data.dt[index] = currenttime;
    
    int16_t tmp;
    if(is_digital){
      tmp = digitalRead(pin_data.dname[index]);
    }else{
      tmp = analogRead(pin_data.dname[index]);
    }
    if ( pin_data.d[index] != tmp ) {
      pin_data.d[index] = tmp;
      is_changed = true;    
    }
  }
}

//开机函数
void setup() {
  
  Serial.begin(115200);
  
  //D1、D2、A0，配置为输入模式，用于接收传感器信号
  pinMode(pin_data.dname[0], OUTPUT);
  pinMode(pin_data.dname[1], OUTPUT);

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

//自循环函数
void loop() {
  ws.cleanupClients();

  bool is_changed = false;
  
  // 如果引脚数据发生变化，就发送给所有客户端
  // 
  if (is_changed) {
    Serial.println("is changed");
    sendPinDatas();
  }  
}
