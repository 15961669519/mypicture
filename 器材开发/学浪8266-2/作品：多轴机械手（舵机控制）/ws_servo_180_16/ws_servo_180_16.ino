#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "ota.h"

const char* ssid = "1024";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

/**
   用一个struct 保存引脚的数据，检测到任何一个引脚发生变化，则将整个struct 发送给客户端
*/
struct PINDATA { 
  /**
   * 对引脚的数量进行修改，必须同步修改下面的所有数据
   * 且需要在web控制界面自行添加对应名称的滑块、onmessage 收到消息时也需要手动添加赋值
   * 该玩法可以兼容8266 mini 芯片、8266 01s 芯片（01s 只有D3、D4引脚，对应io0、io2）
   */
  //引脚数组的长度
  uint8_t dcount = 16;
  
  //可用的引脚编号（此处直接填写IO数字编号，能够兼容所欲8266的开发板）
  const uint8_t dname[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  
  //给每个IO数字编号一个"英文标识"，用于WEB界面和芯片通讯（如果使用mini d1建议使用D开头的编号）
  const String dkey[16] = {"D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","D10","D11","D12","D13","D14","D15"};  //D3和D4，对应01s 芯片的io0、io2
  
  //引脚当前的状态（0-255 对应PWM的功率值）
  uint16_t d[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  const uint8_t servo_count = 16;  //有几个引脚支持舵机？
  const uint8_t servo_pins_index[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};  //舵机引脚的索引！参考dname，可同时控制多个舵机，输入对应引脚的索引即可
  int servo_index[16]  = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};  //舵机对象的默认状态
  
} pin_data;

#include "servo.h";

#include "html.h";
#include "ws.h";

void setup() {
  
  Serial.begin(115200);
  
  //定义LED引脚的默认状态，由于我们引脚数组中，第二个为IO2,所以此处直接赋值给它
  analogWrite(LED_BUILTIN, pin_data.d[1]);

  //连接wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  
  //初始化OAT
  ota_setup();
  
  //启动websocket
  initWebSocket();
  
  //创建HTML页面和请求
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();

  servo_setup();
}

void loop() {
  ws.cleanupClients();
  
  //ota升级监听
  ota_loop();
}
