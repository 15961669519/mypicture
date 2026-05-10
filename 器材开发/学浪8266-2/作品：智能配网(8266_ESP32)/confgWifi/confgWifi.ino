//包含wifi的配网代码
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  wifi_setup();
}


unsigned long lst = 0;
int interval = 1000;

void loop() {
  wifi_loop();

  //定时输出信息
  unsigned long curr = millis();
  if(curr-lst>interval){
    Serial.print("hello ");
    lst = curr;
    
    //判断wifi是否已链接
    if(WiFi.status() == WL_CONNECTED){
      
      //链接到的wifi 名字
      Serial.print(WiFi.SSID());
      Serial.print(" - ");
      
      //设备的局域网IP地址
      Serial.println(WiFi.localIP());
    }
  }
}
