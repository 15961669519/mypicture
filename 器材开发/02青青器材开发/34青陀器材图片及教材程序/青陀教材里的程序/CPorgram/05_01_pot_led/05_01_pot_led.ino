const int potPin = A0;         //设置电位器模块的连接引脚为A0
const int ledPin=3;

void setup() {
  Serial.begin(9600);          //打开串口并设置串口的波特率
}

void loop() {
  int potVal = analogRead(potPin);       //从A0引脚读取模拟值
  Serial.print("Value= ");
  Serial.println(potVal);       //将串口值输送到串口监视器
  potVal=potVal/4;
  analogWrite(ledPin,potVal);
  
}

