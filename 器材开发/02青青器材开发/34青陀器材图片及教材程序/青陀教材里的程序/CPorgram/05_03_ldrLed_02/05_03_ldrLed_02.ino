const int ldrPin = A0; //光敏电阻连接引脚A0
const int ledPin = 3; //LED模拟输出引脚3
int maxVal = 0; //设定当前环境光的最大值
int minVal = 1023; //设定当前环境光的最小值

void setup() {
  Serial.begin(9600);
  analogWrite(ledPin, 255);  //
  for (int i = 0; i < 1000; i++) {  //循环1000次检测当前环境光的最小值和最大值
    int val = analogRead(ldrPin);  //获取光敏电阻电路的返回值
    if (val > maxVal) {  //如果val大于maxVal
      maxVal = val;       // 将maxVal的值更改为val的值
    }
    if (val < minVal) { //如果val小于maxVal
      minVal = val;  // 将minVal的值更改为val的值
    }
    delay(5);，
  }
  analogWrite(ledPin, 0);  // LED灯熄灭，检测结束
  Serial.print("max= ");
  Serial.println(maxVal);   //输出最大值
  Serial.print("min= ");
  Serial.println(minVal);  //输出最小值
}

void loop() {
  int val = analogRead(A0);//获取光敏电阻电路的返回值
  val = constrain(val, minVal, maxVal); //限定范围
  val = map(val, minVal, maxVal, 0, 255); //数据映射
  analogWrite(ledPin, val);  //模拟输出，LED亮度发生变化
  Serial.println(val);  //将映射后的值输出到串口监视器
  delay(100);
}



