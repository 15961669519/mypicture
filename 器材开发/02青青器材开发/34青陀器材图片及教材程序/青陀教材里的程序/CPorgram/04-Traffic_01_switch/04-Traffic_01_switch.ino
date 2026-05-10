const int switchPin = 7;        // 按键开关连接引脚7

void setup() {
  Serial.begin(9600);    // 设置串口波特率为9600
  pinMode(switchPin, INPUT);    //设置引脚7为输入模式
}

void loop() {
  int switchValue = 0;    //定义变量并赋初值为0
  switchValue = digitalRead(switchPin);     //读取引脚7的值
  Serial.print(" Value of switch = ");          //输出到串口监视器
  Serial.println(switchValue);       //将读取的按键值输出到串口监视器
}
