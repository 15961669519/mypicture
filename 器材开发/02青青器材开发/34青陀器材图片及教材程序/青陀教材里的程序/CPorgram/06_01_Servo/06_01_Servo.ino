#include<Servo.h>  //导入舵机库

Servo myServo;  //定义舵机对象实例

const int servoPin=3; //舵机连接D3引脚
const int potPin=A0;  //电位器连接A0引脚

void setup() {
  myServo.attach(servoPin); //指定舵机的控制引脚为D3
}

void loop() {
  int val = analogRead(potPin);   //读取电位器的值
  val = map(val, 0, 1023, 30, 150);  //点电位器的值从0~1023映射到30~150度
  myServo.write(val);   //转动舵机到制定角度
  delay(100);     //延时
}

