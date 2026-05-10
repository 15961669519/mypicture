const int potPin = A0; // 电位器模块连接到模拟输入A0引脚
const int ledPin = 3; // LED灯连接到模拟输出引脚3
int ledStep = 5;  //定义变量，用于保存LED亮度变化的步长值为5
int ledBright = 0;   //定义变量，用于保存LED灯的亮度值
int potVal;

void setup() {
}

void loop() {
  ledBright += ledStep;   //亮度每次递增
  analogWrite(ledPin, ledBright); //LED灯亮度发生变化
  potVal = analogRead(potPin);  //读取电位器的值，范围0~1023
  potVal = potVal / 10;  //值除以10，暂停时间的范围0~102ms
  delay(potVal);  // 暂停，用于调节LED呼吸灯变化的频率
  //本程序的关键，当LED灯最亮或最暗时，通过改变步长值的正负实现呼吸的效果
  if ((ledBright == 255) || (ledBright == 0)) {
    ledStep = -ledStep;   //步长值的数值取反 
  }
}



