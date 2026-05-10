#include <WiFiManager.h>

//用于触发配置网络的针脚
#define TRIGGER_PIN D3

// 配置网络的超时时间（芯片会自动重启）
uint ap_timeout = 120;

//设置阻塞模式 true = 非阻塞模式
bool wm_nonblocking = false;

const char* apName = "AutoConnectAP";

//核心对象
WiFiManager wm;

//进入配置模式的按钮函数
void toConfigButton(){
  // 检查引脚是否进入低电平
  if ( digitalRead(TRIGGER_PIN) == LOW ) {

    //信号持续时间（防止误触发）
    delay(50);

    //50ms后，依然是低电平，我们认为按钮真的被按下了
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      
      //等待3秒
      delay(3000);

      //3秒后还是低电平，则进入配网模式
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        
        //擦除原来的配置
        wm.resetSettings();
        
        //重启设备，会自动进入配网模式
        ESP.restart();
      }
      
      // 如果3秒后，并非是低电平，依然进入配网模式，但是不会擦除历史信息
      Serial.println("Starting config portal");
      
      wm.setConfigPortalTimeout(ap_timeout);
      
      if (!wm.startConfigPortal(apName)) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        ESP.restart();
      } else {
        Serial.println("connected...yeey :)");
      }
    }
  }
}
//配网模式的setup函数
void wifi_setup(){
  
  WiFi.mode(WIFI_STA);
  
  //输出调试信息
  Serial.setDebugOutput(true);
  
  delay(3000);
  Serial.println("\n Starting");

  //用于重置的引脚
  pinMode(TRIGGER_PIN, INPUT);

  //运行模式（阻塞 or 非阻塞）
  if(wm_nonblocking) wm.setConfigPortalBlocking(false);

  //定义配网菜单
  std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
  wm.setMenu(menu);

  // 页面风格（黑暗模式）
  wm.setClass("invert");
  
  //配网超时
  wm.setConfigPortalTimeout(ap_timeout);

  //配网结果
  bool res;
  res = wm.autoConnect(apName);

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } 
  else {
    Serial.println("connected...yeey :)");
  }
}

//配网循环函数
void wifi_loop() {
  //如果是非阻塞模式，会用它来处理配网网页
  if(wm_nonblocking) wm.process();
  
  //监听配网按钮
  toConfigButton();
}
