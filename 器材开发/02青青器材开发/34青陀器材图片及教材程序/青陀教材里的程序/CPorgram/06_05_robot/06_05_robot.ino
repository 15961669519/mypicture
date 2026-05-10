#include <Servo.h> //导入舵机库
#include <Voice.h> //// 导入语音库

const int trigPin = 2;   // 定义超声波Trig引脚连接到D2引脚
const int echoPin = 3; // 定义超声波Echo引脚连接到D3引脚
const int sdaPin = 12; // 语音模块S引脚连接到D12引脚
const int busyPin = 11;  // 语音模块B引脚连接到D11引脚
const int servoPin = A1; //舵机连接A1引脚
const int distMin = 10;
const int distMax = 30;

Voice voice(sdaPin, busyPin); // 定义语音对象实例
Servo myServo;  //定义舵机对象实例

int distance;    // 定义全局变量，保存距离
int preDist = 0;  // 定义全局变量，保存先前距离
int nearOrLeft = 0;   // 状态变量，0-距离不变，静止，1-靠近，2-远离

void setup() {
  Serial.begin(9600);  //设置串口波特率
  pinMode(trigPin, OUTPUT);  //设置D2引脚为输出模式
  pinMode(echoPin, INPUT);  //设置D3引脚为输入模式
  myServo.attach(servoPin); // 关联舵机引脚A1
  myServo.write(90);    // 初始化舵机角度为90度
  delay(200);
}

void loop() {
  getDistance();  // 获取当前距离，通过全局变量distance传递
  Serial.println(distance);  // 将distance输出到串口监视器用于调试
  if (preDist < distance) {  // 当当前距离大于先前距离时，远离
    nearOrLeft = 2;   // 状态变量值为2，表示远离
  }
  else if (preDist > distance) { // 当当前距离小于于先前距离时，靠近
    nearOrLeft = 1;   // 状态变量值为1，表示靠近
  }
  else
    nearOrLeft = 0;  // 静止
  if ((distance == distMin) && (nearOrLeft == 1)) { //靠近并达到反应下限时
    myServo.write(10);  // 舵机转动到10度
    voice.VoiceWord(33);  // 语音播报：欢迎光临
    delay(200);
  }
  else if ((distance == distMax) && (nearOrLeft == 2)) { //远离并达到反应上限时
    myServo.write(170);  // 舵机转动到170度
    voice.VoiceWord(30); // 语音播报：您
    voice.VoiceWord(42);  // 语音播报：慢走
    voice.VoiceWord(40);  // 语音播报：欢迎常来
    delay(200);
  }
  else {    //其他状态时
    myServo.write(90);  //舵机转动到90度，向下
    delay(200);
  }
  preDist = distance;  //保存当前距离，开始下一次检测

}

void getDistance() {
  digitalWrite(trigPin, LOW);   //Trig引脚拉低，准备生成触发脉冲
  delayMicroseconds(2);        //延时2ms
  digitalWrite(trigPin, HIGH);  //Trig引脚拉高
  delayMicroseconds(10);     //延时10ms
  digitalWrite(trigPin, LOW);  //Trig引脚拉低，生成触发脉冲完毕
  distance = pulseIn(echoPin, HIGH) / 58;  //获得距离

}













