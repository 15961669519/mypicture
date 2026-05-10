#include <IRremote.h>  //引用红外库

const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
const int minSpeed = 90;  // 设置马达最小速度
const int maxSpeed = 255; // 设置马达最大速度
const int speedStep = 10; // 设置速度改变的步长值
const int minPower = 680;  // 设置电源电压最低值
const int irPin = A1;   // 红外接收模块连接引脚
int speedPWM = 150; //设置小车运行的初始速度

IRrecv irRecv(irPin);  //定义红外库对象irRecv，并制定数据接收引脚
decode_results irResults;  //定义红外数据对象irResults

void setup() {
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);
  irRecv.enableIRIn();   // 启动红外接收
  Serial.begin(115200);
}

void loop() {
  if (irRecv.decode(&irResults)) {  //接受红外数据并将结果保存到irResults
    Serial.println(irResults.value, HEX);
    switch (irResults.value) {    // 判断红外按键值
      case 0xFF10EF :    //左转
        turnLeft();
        break;
      case 0xFF5AA5 :   //右转
        turnRight();
        break;
      case 0xFF18E7:    //前进
        forward();
        break;
      case 0xFF4AB5:    //后退
        backward();
        break;
      case 0xFF38C7:     //停止
        pause();
        break;
      case 0xFFA25D:     //减速
        speedPWM = ((speedPWM - speedStep) < minSpeed) ? minSpeed : (speedPWM - speedStep);
        break;
      case 0xFF629D:    //加速
        speedPWM = ((speedPWM + speedStep) > maxSpeed) ? maxSpeed : (speedPWM + speedStep);
        break;
    }
    if (analogRead(A7) > minPower) {   // 判断电池电压
      analogWrite(leftSpeed, speedPWM); //设定左侧电机的速度
      analogWrite(rightSpeed, speedPWM); //设定右侧电机的速度
    }
    else {
      analogWrite(leftSpeed, 0); //设定左侧电机的速度
      analogWrite(rightSpeed, 0); //设定右侧电机的速度
      digitalWrite(powerLed，HIGH);
      Serial.println("Battery Warning!");
    }
    irRecv.resume(); // 接收下一个值
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
