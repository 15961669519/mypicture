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
  uint8_t dcount = 6;
  
  //可用的引脚编号（此处直接填写IO数字编号，能够兼容所欲8266的开发板）
  const uint8_t dname[6] = {0,2,14,12,13,15};
  
  //给每个IO数字编号一个"英文标识"，用于WEB界面和芯片通讯（如果使用mini d1建议使用D开头的编号）
  const String dkey[6] = {"D3", "D4", "D5","D6","D7","D8"};  //D3和D4，对应01s 芯片的io0、io2
  
  //引脚当前的状态（0-255 对应PWM的功率值）
  uint16_t d[6] = {0,0,0,0,0,0};
  
  const uint8_t timer_index = 2;  //支持定时的引脚索引（对应dname数组的索引，0表示第一个引脚，2表示第三个引脚。）
  uint16_t timer_pwm = 255; //定时器开始工作后，引脚的状态（通过web修改引脚的值以后，会同步更新）
  uint16_t timer_pwm_stop = 0;  //定时关闭后引脚的状态
  
  String *dt; //定时数组，内部结构为时:分 {11:25,12:30...}
  uint8_t dtcount = 0;  //定时器数组长度，程序会自动维护，无需设置。
  uint8_t duration= 0;  //定义的持续时间，程序会自动维护，无需设置。
  unsigned long duration_timer = 0; //记录触发的时间起点，用于计算持续时间

  //ntp 的时间和日期
  String localTime;
  String localDate;

  //ntp 的时分秒，用于比较定时器
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  
} pin_data;

#include "html.h";
#include "ws.h";

#include "ntptime.h"

unsigned long last_ntp = 0;     //上一次ntp的检查时间
const long interval_ntp = 3000; //每1秒更新一次NTP时间，并检查定时器是否需要启动

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

  //初始化时间
  initNtp();
  
  //初始化OAT
  ota_setup();
  
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
  
  //获取单片机启动至今的毫秒数
  unsigned long currentMillis = millis();
    
  //ota升级监听
  ota_loop();
  
  //间隔一定时间后，更新一次时间，并同步检查定时器是否需要触发
  if (currentMillis - last_ntp >= interval_ntp) {

    //获得单片机开机至今的微秒数
    last_ntp = currentMillis;

    //更新时间
    loopNtp();
    
    //检查定时器（如果持续时间超过0分钟，表示定时器启用）
    if(pin_data.duration>0){
      
      //说明已经开始工作了
      if(pin_data.duration_timer>0){
        
        Serial.print("计时器工作中:");
        Serial.println(pin_data.duration_timer);
        
        //判断是否满足停止条件（表达式：当前时间-启动时间 > 持续时间）
        if(currentMillis-pin_data.duration_timer > pin_data.duration*1000*60 ){
          
          //恢复引脚到默认状态
          pin_data.d[pin_data.timer_index] = pin_data.timer_pwm_stop;
          execPin(pin_data.timer_index,pin_data.timer_pwm_stop);
          
          //重置启动时间
          pin_data.duration_timer = 0;
          
          Serial.print("计时器已重置");
          Serial.println(pin_data.duration_timer);          
        }
      }else{  //计时器还没有开始工作，进入触发验证过程
        String tmp;

        //循环启动时间数组，逐一检查是否满足触发条件（时间是从凌晨到半夜正序排列的，比如 03:25,05:00...）
        for(uint8_t i=0;i<pin_data.dtcount;i++){
          
          //先从数组中取出一个时间，保存到临时变量
          tmp = pin_data.dt[i];

          //删除时间中的:号。
          tmp.replace(":","");

          //如果当前时间 == 定时器时间，则准备启动开关
          if(pin_data.hour*100+pin_data.minute == tmp.toInt()){
            
            Serial.println("触发定时器");
            
            //记录启动时间点
            pin_data.duration_timer = currentMillis;
            
            //必须开关处于【关闭】状态，才执行启动（避免重复启动）
            if(pin_data.d[pin_data.timer_index] != pin_data.timer_pwm){
              
              //更新开关状态
              pin_data.d[pin_data.timer_index] = pin_data.timer_pwm;
              execPin(pin_data.timer_index,pin_data.timer_pwm);
            } 
            
            //已经匹配到了时间的情况下，不需要继续匹配，直接跳出循环
            break;
          }
        }
      }
    }
  }
  
}
