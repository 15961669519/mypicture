const int TrigPin = 2;   // 定义超声波Trig引脚连接到D2引脚
const int EchoPin = 3; // 定义超声波Echo引脚连接到D3引脚

void setup() {
  Serial.begin(9600);  //设置串口波特率
  pinMode(TrigPin, OUTPUT);  //设置D2引脚为输出模式
  pinMode(EchoPin, INPUT);  //设置D3引脚为输入模式

}

void loop() {
  float distance;   // 定义变量，保存距离
  digitalWrite(TrigPin, LOW);   //Trig引脚拉低，准备生成触发脉冲
  delayMicroseconds(2);        //延时2ms
  digitalWrite(TrigPin, HIGH);  //Trig引脚拉高
  delayMicroseconds(10);     //延时10ms
  digitalWrite(TrigPin, LOW);  //Trig引脚拉低，生成触发脉冲完毕
  distance = pulseIn(EchoPin, HIGH) / 58.0;  //获得距离
  Serial.print(distance);     //串口输出超声波检测的距离
  Serial.println("cm");

}


