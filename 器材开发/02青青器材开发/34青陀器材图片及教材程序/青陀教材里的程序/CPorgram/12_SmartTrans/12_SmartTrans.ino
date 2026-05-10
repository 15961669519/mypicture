//***********************************
//
//  智能运输器小车控制程序
//
//  功能：通过手机蓝牙控制小车运动，收集器的转向变化。
//
//  作者：行者无疆（微信号：XZFY_G0）
//
//  说明：。任何人不得用于商业用途。此外本人不承担因使用该软件而产生的不良后果。
//
//***********************************

// 定义所应用库的头文件

#include <Servo.h>                 // 舵机库头文件

// 定义 引脚
const byte serPinLeft = 12;  //舵机左侧引脚
const byte serPinRight = 13;  //舵机右侧引脚

const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
int speedPWM = 150; //设置小车运行的初始速度


Servo serLeft;                  // 新建一个舵机对象myServo
Servo serRight;


byte readBuffer[5];
unsigned int Command;
byte slideValue;

byte speedStep = 15;    //小车的速度调整幅度
byte minSpeedPwm = 90; //小车的最慢速度
byte maxSpeedPwm = 255; //小车的最快速度

void setup() {
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);

  //初始化串口
  Serial.begin(9600);

  //初始化舵机
  serLeft.attach(serPinLeft);
  serRight.attach(serPinRight);

  serLeft.write(160);
  serRight.write(20);
}

void loop() {

  Command = 0;
  getCommand();   //读取蓝牙和红外遥控器的控制命令

  Serial.println("==");
  Serial.println(Command, HEX);

  analogWrite(leftSpeed, speedPWM);
  analogWrite(rightSpeed, speedPWM);

  switch (Command) {

    case 0x4141 :                    // 收集装置放下
      serLeft.write(160);
      serRight.write(20);
      delay(500);
      break;
    case 0x4242:                     // 收集装置抬起
      serLeft.write(60);
      serRight.write(120);
      delay(500);
      break;
    case 0x4c53 :         //左转
      rotateLeft();
      break;
    case 0x5253 :         // 右转
      rotateRight();
      break;
    case 0x5553:                 //前进
      forward();
      break;
    case 0x4453:                 //后退
      backward();
      break;
    case 0x4452:                 //停止
      pause();
      break;
    case 0x444c:                 // 减速
      speedPWM -= speedStep;
      speedPWM = (speedPWM < minSpeedPwm) ? minSpeedPwm : speedPWM;
      break;
    case 0x554c :                 // 加速
      speedPWM += speedStep;
      speedPWM = (speedPWM > maxSpeedPwm) ? maxSpeedPwm : speedPWM;
      break;
    case 0x5552 :                 // 全速
      speedPWM = maxSpeedPwm;
      break;
  }
}

//*******************************
//  功能：读取蓝牙的控制命令
//  参数：无
//  返回值：通过全局变量传递命令。
//          Command     返回按键的键值
//          slideValue  返回滑动条的键值
//*******************************
void getCommand() {
  char flag = 1;
  uint8_t num = 0;

  while (flag) {
    if (Serial.available()) {
      uint8_t tempBuffer = Serial.read();
      readBuffer[num] = tempBuffer;
      Serial.println(tempBuffer, HEX);
      num++;

      if (tempBuffer == 0x0D) {
        Command = (readBuffer[0] << 8) | (readBuffer[1]);
        flag = 0;
        Serial.print(Command, HEX);
        if (num == 5) {
          slideValue = readBuffer[2];
        }
      }
    }
  }
}

//===============
// 前进
//===============
void forward() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 后退
//===============
void backward() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 1);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 1);
}
//===============
// 左转
//===============
void turnLeft() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 右转
//===============
void turnRight() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 0);
}
//===============
// 原地左转
//===============
void rotateLeft() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 1);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 原地右转
//===============
void rotateRight() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 1);
}
//===============
// 停止
//===============
void pause() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 0);
}

