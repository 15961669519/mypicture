const int switchPin = 6; // 按键开关连接引脚6
const int buzzerPin = 10;  //蜂鸣器模块连接引脚10

void setup() {
  pinMode(switchPin, INPUT);    //设置引脚6位输入模式

}
void loop() {
  int val = digitalRead(switchPin); // 获取按键开关的值
  if (val == 0) {           //按键按下，返回值为0
    tone(buzzerPin, 532); // duo
    delay(100);
    tone(buzzerPin, 587); // re
    delay(100);
    tone(buzzerPin, 659); // mi
    delay(100);

    //    for (int i = 200;i <= 1000; i += 10) {
    //      tone(buzzerPin, i);
    //      delay(10);
    //    }
    //    for (int i = 1000; i >= 200; i -= 10) {
    //      tone(buzzerPin, i);
    //      delay(10);
    //    }
  }
  else {
    noTone(10);
  }

}




