#include <Voice.h>  // 调用头文件

const int sdaPin = 12;
const int busyPin = 11;
const int switchPin = 2;

Voice myVoice(sdaPin, busyPin);  // 定义对象实例

void setup() {
  // put your setup code here, to run once:
  pinMode(switchPin, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  int val = digitalRead(switchPin);
  if (val == 0) {
    myVoice.VoiceWord(73);  // 播报对应地址的语音内容
    myVoice.VoiceNum(73);   // 播报数字
  }
}
