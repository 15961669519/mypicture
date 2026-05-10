const int rLedPin1 = 4;  //主路红色LED灯连接引脚4
const int yLedPin1 = 5;  //主路黄色LED灯连接引脚5
const int gLedPin1 = 6;  //主路绿色LED灯连接引脚6
const int rLedPin2 = 8;   //人行道红色LED灯连接引脚8
const int gLedPin2 = 10; //人行道绿色LED灯连接引脚10

const int switchPin = 2;  //按键开关连接引脚2
const int buzzerPin = 12;   //蜂鸣器连接引脚12

void setup() {
  pinMode(rLedPin1, OUTPUT);     //设置引脚4为输出模式
  pinMode(yLedPin1, OUTPUT);     //设置引脚5为输出模式
  pinMode(gLedPin1, OUTPUT);     //设置引脚6为输出模式
  pinMode(rLedPin2, OUTPUT);     //设置引脚9为输出模式
  pinMode(gLedPin2, OUTPUT);   //设置引脚11为输出模式
  pinMode(switchPin, INPUT);         // 设置引脚2为输入模式
  pinMode(buzzerPin, OUTPUT);      // 设置引脚12为输出模式

}
void loop() {
  int switchValue = 0;      //定义变量并赋初值为0
  switchValue = digitalRead(switchPin);      //读取按键引脚2的值
  if (switchValue == 1) {        //判断键值等于0，执行下面的程序
    digitalWrite(gLedPin1, HIGH);       //主路绿色LED亮
    digitalWrite(yLedPin1, LOW);       //主路黄色LED灭
    digitalWrite(rLedPin1, LOW);          //主路红色LED灭
    digitalWrite(gLedPin2, LOW);       //人行道绿色LED灭
    digitalWrite(rLedPin2, HIGH);         //人行道红色LED亮
  }
  else {        //按键按下，键值为1,将执行下面的语句
    digitalWrite(gLedPin1, LOW);          //主路绿色LED灭
    digitalWrite(yLedPin1, HIGH);         //主路黄色LED亮
    delay(2000);                             //等候2秒
    digitalWrite(yLedPin1, LOW);         //主路黄色LED灭
    digitalWrite(rLedPin1, HIGH);           //主路红色LED亮
    digitalWrite(rLedPin2, LOW);           //人行道红色LED灭
    digitalWrite(gLedPin2, HIGH);         //人行道绿色LED亮
    delay(6000);                            //等候6秒
    // 人行道的绿色LED闪烁，同时蜂鸣器响。时间长度2秒
    for (int i = 1; i <= 5; i++) {
      digitalWrite(gLedPin2, HIGH);         //人行道绿色LED亮
      tone(buzzerPin, 500);        //调用蜂鸣器发声函数，同时延时100ms
      delay(200);
      digitalWrite(gLedPin2, LOW);         //人行道绿色LED灭
      noTone(buzzerPin);
      delay(200);
    }
  }
}


