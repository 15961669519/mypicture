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
  uint8_t dname[9] = {D1, D2, D3, D4, D5, D6, D7, D8, A0};
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
  uint16_t d[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  //上一次触发时间
  unsigned long dt[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  int8_t temperature = -99;
  int8_t humidity = -99;
} pin_data;

#include "sht30.h";
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
  
  //设置默认的电平
  for (uint8_t i=4; i<8; i++) {
    pinMode(pin_data.dname[i], OUTPUT);
  }
  
  //D1、D2、A0，配置为输入模式，用于接收传感器信号
  pinMode(pin_data.dname[0], INPUT);
  pinMode(pin_data.dname[1], INPUT);
  pinMode(pin_data.dname[8], INPUT);

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

  sht30_setup();
}

// 自动触发的代码
// 每隔50毫秒执行一次检查，可实现自动触发，比如根据温度的值触发D5的状态
unsigned long auto_last_time = 0;
int auto_interval = 100;

//自循环函数
void loop() {
  ws.cleanupClients();

  //读取温湿度到数据
  sht30();
  
  //将得到的数据，集中保存到pin_data，方便发给控制器
  pin_data.temperature = sht_data.temperature;
  pin_data.humidity = sht_data.humidity;


  /**
   * 检查 D1 D2 A0 的引脚状态，并发给客户端
   * 这三个引脚被设为INPUT，主要用于接收传感器之类的数据
   * 当然，你也可以接实体按钮去做点什么。
   */
  bool is_changed = false;
  check_pin(0, is_changed);
  check_pin(1, is_changed);
  check_pin(8, is_changed, false, 100);

  /**
   * 自动化触发的代码
   * 为了防止频繁触发，我们每隔一定时间检查一次传感器的数据
   * 根据判断条件，修改指定引脚的状态，由于我们支持PWM，因此高电平=255，低电平=0
   *    需要使用analogWrite 函数写入（以前都讲过例子啦）
   */

  // 根据设定的频率进行检查，代码含义参考（BlinkwithoutDelay）例子
  unsigned long current_time = millis();
  if(current_time - auto_last_time > auto_interval){
    auto_last_time = current_time;
    
    //可以自己编写其他控制条件，也可以多条件组合
//    if(pin_data.temperature>32){  //如果温度超过设定值
    if(pin_data.d[0]==1){  //如果D1引脚为高电平
      // 如果D6引脚状态不是高电平，则修改为高电平，并通知客户端
      if(pin_data.d[5] < 1){
        analogWrite(pin_data.dname[5],255);
        pin_data.d[5] = 255;
        is_changed = true;
      }
      
    }else{  //如果温度不满足设定值，执行反向操作
      
      //如果D5引脚状态不是低电平，则修改为低电平，并通知客户端
      if(pin_data.d[5] != 0){
        analogWrite(pin_data.dname[5],0);
        pin_data.d[5] = 0;
        is_changed = true;
      }      
    }
  }
  
  // 如果引脚数据发生变化，就发送给所有客户端
  // 
  if (is_changed) {
    Serial.println("is changed");
    sendPinDatas();
  }  
}
