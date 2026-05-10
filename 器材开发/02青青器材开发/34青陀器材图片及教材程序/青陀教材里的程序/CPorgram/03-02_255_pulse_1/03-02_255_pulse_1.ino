void setup() {
  int i = 255;      // 定义整数型变量
  byte j = 255;     // 定义字节型变量
  char k = 32;      // 定义字符型变量
  Serial.begin(9600);   // 设置串口波特率
  Serial.println(i);
  Serial.println(j);
  Serial.println(k);
  i = i + 1;
  j ++;
  k += 16;
  Serial.println(i);
  Serial.println(j);
  Serial.println(k);
  Serial.println(k + 16);
}

void loop() {
}


