/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade
*/

int led = D1;           // 输出PWM信号的针脚
int brightness = 0;    // 初始亮度
int fadeAmount = 5;    // 每次递增多少亮度值

// the setup routine runs once when you press reset:
void setup() {
  // 定义针脚为输出模式
  pinMode(led, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // 向针脚写入亮度信息，analogWrite 会自动转为PWM输出
  analogWrite(led, brightness);

  // 亮度增加一级
  brightness = brightness + fadeAmount;

  // 如果亮度超出了范围，则递减（亮度下降）
  if (brightness <= 0 || brightness >= 1023) {
    fadeAmount = -fadeAmount;
  }
  // 每30毫秒变化一次
  delay(30);
}
