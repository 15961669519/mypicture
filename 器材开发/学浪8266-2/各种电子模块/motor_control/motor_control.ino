void setup() {
  //引脚设为输出状态
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  //设为默认低电平
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW);
}

void loop() {
  //正转
  digitalWrite(D1,HIGH);
  digitalWrite(D2,LOW);
  delay(2000);
  //反转
  digitalWrite(D1,LOW);
  digitalWrite(D2,HIGH);
  delay(2000);
  //刹车
  digitalWrite(D1,HIGH);
  digitalWrite(D2,HIGH);
  delay(1000);
  //待机
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW);
  delay(1000);

  //正转逐步加速
  for(int i=0;i<255;i++){
    analogWrite(D1,i);
    delay(50);
  }

  //反转逐步加速
  digitalWrite(D1,LOW);
  for(int i=0;i<255;i++){
    analogWrite(D2,i);
    delay(50);
  }

  delay(1000);
}
