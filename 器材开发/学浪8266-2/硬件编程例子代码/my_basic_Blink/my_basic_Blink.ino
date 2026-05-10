// 开机函数（每次开机执行一次）
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);// 设定针脚为输出模式
}

// 开机后的循环工作函数
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  //开灯（针脚设为高电平）
  delay(1000);    //等1秒
  digitalWrite(LED_BUILTIN, LOW);   //关灯（针脚设为低电平）
  delay(1000);    //等一秒
}
