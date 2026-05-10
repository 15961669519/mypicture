const int potPin = A0; // 电位器模块连接到模拟输入A0引脚
const int ledPin = 3;  // LED灯连接到模拟输出引脚3

void setup() {
}

void loop() {
  int potVal;
  for (int i = 0; i <= 255; i += 5) {  //LED灯由暗变亮，步长为5
    analogWrite(ledPin, i);    //模拟输出，LED灯亮度发生变化
    potVal = analogRead(potPin);  //读取电位器的值，范围0~1023
    potVal = potVal / 10;    //值除以10，暂停时间的范围0~102ms
    delay(potVal);  // 暂停，用于调节LED呼吸灯变化的频率
  }
  for (int i = 255; i >= 0; i -= 5) {  //LED灯由亮变暗，步长为5
    analogWrite(ledPin, i);    //模拟输出，LED灯亮度发生变化
    potVal = analogRead(potPin);  //读取电位器的值，范围0~1023
    potVal = potVal / 10;  //值除以10，暂停时间的范围0~102ms
    delay(potVal);  // 暂停，用于调节LED呼吸灯变化的频率
  }
}

