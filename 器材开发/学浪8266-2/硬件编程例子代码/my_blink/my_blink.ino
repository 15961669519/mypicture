

int led = D1;           // 输出PWM信号的针脚
int brightness = 0;    // 初始亮度
int fadeAmount = 5;    // 每次递增多少亮度值

void setup() {
  // 定义针脚为输出模式
  pinMode(led, OUTPUT);
}

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
