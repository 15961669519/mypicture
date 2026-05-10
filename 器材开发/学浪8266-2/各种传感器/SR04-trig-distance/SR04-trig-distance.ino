/**
   1655 超声波探头适合20-600厘米的测距
   近距离测距还有 SR04 可选，用法一致
*/
float cm; //厘米
float temp; //信号持续时间的原始值


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("...");
  Serial.println("ok");

  //设置引脚工作模式
  pinMode(D1, OUTPUT);
  pinMode(D2, INPUT);

  //设置引脚的默认电平
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);

  //略等一下
  delay(100);
}

void loop() {

  //先将trig设为低电平（防止默认电平异常）
  digitalWrite(D1, LOW);
  delayMicroseconds(2);

  //设为高电平，10us之后，回到低电平（这是传感器要求的信号格式）
  digitalWrite(D1, HIGH);
  delayMicroseconds(10);
  digitalWrite(D1, LOW);

  //通过pulseIn 计算 D2 高电平的信号持续时间
  temp = float(pulseIn(D2, HIGH));

  /**
     声速温度公式：c=(331.45+0.61t/℃)m•s-1 (其中 330.45 是在 0℃）
     0℃声速： 330.45M/S
    20℃声速： 342.62M/S
    40℃声速： 354.85M/S
    0℃-40℃声速误差 7 左右。
    实际应用，如果需要精确距离值，必须要考虑温度影响，做温度补偿。
  */
  //用公式得到信号持续时间对应的厘米数
  //cm = (temp * 17) / 1000; 
  //cm = temp/58;
  cm = temp * 0.0343 / 2; //信号往返时间*空气传播速度（343米每秒）/2
  //输出结果
  Serial.print("Echo =");
  Serial.print(temp);
  Serial.print(" | | Distance = ");
  Serial.print(cm);
  Serial.println("cm");
  delay(100);
}
