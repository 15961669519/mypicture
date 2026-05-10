#include "ESP8266_ISR_Servo.h"

//0度（脉冲最小持续时间）（毫秒）
#define MIN_MICROS      544

//180度
#define MAX_MICROS      2500

//舵机列表
int servo_index[1]  = {-1};
  
void setup() {
  
  pinMode(LED_BUILTIN,OUTPUT);

  //定义舵机对象
  servo_index[0] = ISR_Servo.setupServo(D6, MIN_MICROS, MAX_MICROS);

  //控制舵机回0度
  ISR_Servo.setPosition(servo_index[0], 0);
}

void loop() {
  
  delay(5000);
  digitalWrite(LED_BUILTIN,LOW);
  ISR_Servo.setPosition(servo_index[0], 180); //转到指定度数
  
  delay(5000);
  digitalWrite(LED_BUILTIN,HIGH);
  ISR_Servo.setPosition(servo_index[0], 0);
}
