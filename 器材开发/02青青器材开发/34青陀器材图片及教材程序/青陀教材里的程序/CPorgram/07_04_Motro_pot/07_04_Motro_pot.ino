const int potPin = A0;  // 电位器连接A0引脚
const int switchPin = 2;     // 按键模块连接D2引脚
const int speedPin = 6;   // 马达速度控制引脚
const int ledPin = 13;    // 马达启停指示LED引脚D13
boolean  motorEnable = false; // 定义马达启停状态，false停止，true启动
int switchState = 1;   // 保存当前按键模块的返回值
int preSwitchState = 1;  // 保存先前按键模块的返回值
int motorSpeed;        // 保存马达的速度的PWM值
int potVal;            // 保存电位器模块的返回值

void setup()
{
  pinMode(switchPin, INPUT);   // 设置D2引脚为输入
  pinMode(ledPin, OUTPUT);  // 设置D13引脚为输出
}

void loop() {
  motorSpeed = analogRead(potPin) / 4;  // 定位器发的返回值除以4，设定马达转速的PWM值
  switchState = digitalRead(switchPin); // 获取当前按键的值
  delay(20);                            // 延时消抖
  if (switchState != preSwitchState) {  // 如果当前按键值和先前值不等，按键状态发生变化
    if (switchState == HIGH) {          // 按键模块释放状态
      motorEnable = !motorEnable;       // 马达状态反转
    }
  }
  if (motorEnable) {                    // 马达处于启动状态
    analogWrite(speedPin, motorSpeed);  // 输出PWM，启动马达并控制转速
    digitalWrite(ledPin, HIGH);         // 指示灯点亮
  }
  else {
    analogWrite(speedPin, 0);           // 马达停止
    digitalWrite(ledPin, LOW);          // 指示灯熄灭
  }
  preSwitchState = switchState;         // 保存当前状态，开始下一次检测
}
