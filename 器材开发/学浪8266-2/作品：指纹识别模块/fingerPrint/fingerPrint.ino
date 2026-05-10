/**
 * 1024编程实验室 王亮编写
 * 用于物联网爱好者课程，仅服务于购买课程的同学们学习之用。
 */
#include "fpm383.h"

#define FP D3   //指纹中断引脚
#define CT D5   //单片机控制开关的引脚

ICACHE_RAM_ATTR void InterruptOut() {
  TouchState = 1;
  detachInterrupt(digitalPinToInterrupt(FP));
}

void setup() {

  //硬串口
  Serial.begin(115200);

  //软串口（指纹通信串口）
  Serial2.begin(57600);

  while (!Serial) {
    delay(100);
  }
  Serial.println("Serial ok");

  while (!Serial2) {
    delay(100);
  }
  Serial.println("Serial2 ok");

  //指纹中断引脚
  pinMode(FP, INPUT);

  //信号输出引脚（可以接控制器）
  pinMode(CT,OUTPUT);
  digitalWrite(CT,LOW);

  //指纹触摸中断函数
  attachInterrupt(digitalPinToInterrupt(FP), InterruptOut, RISING);
  
  delay(500);
  
  //使指纹模块进入休眠
  fsleep();
  
  //显示当前数据库中有多少指纹
  getFingerTotalFun();
}

void loop() {

  //接受"串口监视器的命令" 1=注册 2=清空已有指纹
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  //串口输入1，进入注册流程 ASCII
        Serial.println("进入注册流程>>>>>>>>>>>>>>>");
        autoReg();
        Serial.println("注册完毕");
        break;
      case 50:  //串口输入2，进入删除流程
        Serial.println("进入删除流程>>>>>>>>>>>>>>>");
        if (deleteFingerFun()) {
          Serial.println("删除成功");
        } else {
          Serial.println("删除失败");
        }
        break;
    }

    //这将触发后面的指纹休眠命令
    TouchState = 1;
  }
  
  //指纹验证
  if (TouchState) {
    Serial.println("进入验证流程>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    LEDControl(BlueLight);
    
    //可以在这里增加代码
    if (checkFingerFun()) {
      Serial.println("验证通过=====");
      
      digitalWrite(CT,HIGH);
      LEDControl(GreenLight);
      
      delay(1000);
      digitalWrite(CT,LOW);
    } else {
      Serial.println("无效指纹=====");
      
      digitalWrite(CT,LOW);
      LEDControl(RedLight);
    }
  }

  //让指纹回去休眠
  if (TouchState) {
    fsleep();
    LEDControl(OffLight);
    TouchState = 0;
    attachInterrupt(digitalPinToInterrupt(FP), InterruptOut, RISING);
  }
}
