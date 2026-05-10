#include <Servo.h>   // 导入舵机库
#include <IRremote.h>   // 导入红外库

const int RECV_PIN = A0;  // 定义红外模块连接引脚A0
const int servoPin = 12;     // 红色LED灯连接D12引脚
const int ledPin = 13;      // 马达启停指示LED引脚D13
const int speedPin = 6;    // 马达速度控制引脚
const int minSpeed = 80;   // 马达最小速度
boolean swingFlag = false; // 舵机状态变量：false-90度停止，true-摇摆
boolean motorFlag = false; // 马达状态变量：false-停止，true-转动
int serAngle = 90;  // 舵机转动角度，初始值为90度
int motorSpeed = 100;  // 马达速度，初始值为100
int angleStep = 1;   // 舵机摆动角度增量


IRrecv irrecv(RECV_PIN);  // 定义红外类库对象实例
decode_results results;  // 定义红外接收解码对象实例
Servo myServo;            // 定义舵机对象示例

void setup()
{
  Serial.begin(9600);   // 设置窗口波特率
  irrecv.enableIRIn();   // 启动红外接收
  myServo.attach(servoPin); // 指定舵机关联引脚D12
  myServo.write(serAngle);  // 舵机转动到初始位置
  pinMode(ledPin, OUTPUT);   // 设置D13引脚为输出

}

void loop() {
  if (irrecv.decode(&results)) {   // 如果接收到红外数据，将解码结果保存到results
    Serial.println(results.value, HEX);  // 将解码结果输出到串口监视器
    switch (results.value) {        // 比较红外按键值
      case 0xFFA25D : // "CH-"键
        if (!swingFlag)    // 当不在摇摆状态时
          serAngle = (serAngle < 5) ? 0 : serAngle - 5;   // 角度减5
        break;         // 当不在摇摆状态时
      case 0xFFE21D :  // "CH+"键
        if (!swingFlag)
          serAngle = (serAngle > 175) ? 180 : serAngle + 5;  // 角度加5
        break;
      case 0xFF629D : // "CH"键
        swingFlag = !swingFlag;  // 归位/摇摆状态切换
        break;
      case 0xFFE01F :  // "-"键
        motorSpeed = ((motorSpeed - 10) < minSpeed) ? minSpeed : motorSpeed - 10;  // 速度减10
        break;
      case 0xFFA857 :  // "+"键
        motorSpeed = ((motorSpeed + 10) > 255) ? 255 : motorSpeed + 10; // 速度加10
        break;
      case 0xFF906F :  // "EQ"键
        motorFlag = !motorFlag;  // 马达启/停状态切换
        break;
    }
    irrecv.resume(); // 清空缓存，接收下一个红外数据
  }
  if (motorFlag)  // 当马达启动时
    analogWrite(speedPin, motorSpeed);  //输出PWM，启动马达并控制转速
  else {
    analogWrite(speedPin, 0);  // 马达停止
    swingFlag = false;        // 舵机状态变量为false，停止摆动
  }
  digitalWrite(ledPin, motorFlag);  // 指示灯状态亮/灭
  if (swingFlag) {     // 当舵机摆动时
    serAngle += angleStep;  // 舵机角度增加
    if ((serAngle == 0) || (serAngle == 180))  // 当舵机摆到两端时
      angleStep = -angleStep;   // 舵机摆动角度增量取反
    delay(50);
    myServo.write(serAngle);  // 转动舵机到增量改变后的角度
  }
  else
    myServo.write(serAngle);    // 舵机停留在指定角度
}

