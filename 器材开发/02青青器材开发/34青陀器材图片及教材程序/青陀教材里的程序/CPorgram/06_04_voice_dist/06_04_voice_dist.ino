#include <Voice.h>  // 导入语音库
const int trigPin = 2;   // 定义超声波Trig引脚连接到D2引脚
const int echoPin = 3; // 定义超声波Echo引脚连接到D3引脚
const int sdaPin = 12; // 语音模块S引脚连接到D12引脚
const int busyPin = 11;  // 语音模块B引脚连接到D11引脚

Voice myVoice(sdaPin, busyPin);  // 定义对象实例

void setup() {
  Serial.begin(9600);  //设置串口波特率
  pinMode(trigPin, OUTPUT);  //设置D2引脚为输出模式
  pinMode(echoPin, INPUT);  //设置D3引脚为输入模式
}

void loop() {
  int distance;
  distance = getDistance();
  myVoice.VoiceWord(51);  // 播报对应地址的语音内容
  myVoice.VoiceNum(distance);   // 播报数字
  myVoice.VoiceWord(53);  // 播报对应地址的语音内容
  myVoice.VoiceWord(52);  // 播报对应地址的语音内容
}
// 将超声波
int  getDistance() {
  int dist; // 定义全局变量，保存距离
  digitalWrite(trigPin, LOW);   //Trig引脚拉低，准备生成触发脉冲
  delayMicroseconds(2);        //延时2ms
  digitalWrite(trigPin, HIGH);  //Trig引脚拉高
  delayMicroseconds(10);     //延时10ms
  digitalWrite(trigPin, LOW);  //Trig引脚拉低，生成触发脉冲完毕
  dist = pulseIn(echoPin, HIGH) / 58;  //获得距离
  Serial.print(dist);     //串口输出超声波检测的距离
  Serial.println("cm");
  return dist;
}

