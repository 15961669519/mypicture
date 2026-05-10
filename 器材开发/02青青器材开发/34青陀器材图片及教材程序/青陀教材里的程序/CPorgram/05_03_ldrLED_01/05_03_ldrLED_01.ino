const int maxVal = 1023; //设定当前环境光的最大值
const int minVal = 300; //设定当前环境光的最小值
const int ldrPin = A0; //光敏电阻连接引脚A0
const int ledPin = 3; //LED模拟输出引脚3

void setup() {
  Serial.begin(9600);  //设置串口波特率为9600
}

void loop() {
  int ldrVal = analogRead(ldrPin); //获取光敏电阻电路的返回值
  ldrVal=constrain(ldrVal,minVal,maxVal);
  int ledVal = map(ldrVal, minVal, maxVal, 0, 255); //将返回值映射到0~255之间对应值
  Serial.print("Value: ");
  Serial.print(ldrVal);      //将光敏电阻返回值输出到串口监视器
  Serial.print(" --  ");
  Serial.println(ledVal);      //将映射后的值输出到串口监视器
  analogWrite(ledPin, ledVal); //模拟输出，LED亮度发生变化
}

