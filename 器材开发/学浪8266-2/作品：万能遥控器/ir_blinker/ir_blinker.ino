#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

//定义发射管所在针脚
const uint16_t kIrLed = D6;

//实例对象
IRsend irsend(kIrLed);

/*************以上是ir的设置******************/

#define BLINKER_WIFI
#include <Blinker.h>

char auth[] = "5fa01417da11";
char ssid[] = "1024";
char pswd[] = "12345678";

#include "btn.h"


void setup()
{
    // 初始化串口
    Serial.begin(115200);

    //定义调试信息的输出途径，Serial = 串口
    BLINKER_DEBUG.stream(Serial);
    
    //在串口监视器输出调试信息
    BLINKER_DEBUG.debugAll();

    // 初始化blinker（联网就靠它了）
    Blinker.begin(auth, ssid, pswd);
    
    //绑定按键
    BtnUp.attach(btn_up);
    
    //绑定按键
    BtnDown.attach(btn_down);

    //绑定按键
    BtnPower.attach(btn_power);
    
    //发射对象的准备工作
    irsend.begin();
}

void loop() {
  //点灯的loop函数
  Blinker.run();
}
