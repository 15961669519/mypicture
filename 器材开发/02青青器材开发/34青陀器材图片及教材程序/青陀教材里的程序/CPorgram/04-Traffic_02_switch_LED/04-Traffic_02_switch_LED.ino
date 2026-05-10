const int ledPin = 4;       // LED灯连接引脚4
const int switchPin = 8;        // 按键开关连接引脚7

void setup() {
  pinMode(ledPin, OUTPUT);      //设置引脚4为输出模式
  pinMode(switchPin, INPUT);           //设置引脚7为输入模式
  Serial.begin(9600);    // 设置串口波特率为9600
}

void loop() {
  int switchValue = 0;    //定义变量并赋初值为0
  switchValue = digitalRead(switchPin);     //读取引脚7的值
  Serial.println(switchValue);       //将读取的按键值输出到串口监视器
  if (switchValue == 0) {              //判断键值等于0，执行下面的程序
    digitalWrite(ledPin, LOW);       //熄灭LED

  }
  else {  //判断键值不为0,将执行下面的语句
    digitalWrite(ledPin, HIGH);       //点亮LED
  }
}

