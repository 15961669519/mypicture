void setup() {
  for (int i = 2; i <= 8; i++) {
    pinMode(i, OUTPUT);
  }
}

void loop() {
  for (int i = 5; i >= 1; i--) {
    ledOn(100);
    ledOff(100);
  }
  for (int i = 3; i >= 1; i--) {
    fromLeftToRightS(50 * i);
    ledOff(10);
    fromRightToLeftS(50 * i);
    ledOff(10);
  }
  for (int i = 3; i >= 1; i--) {
    fromLeftToRightS(50);
    fromRightToLeftS(50);
  }
  for (int i = 3; i >= 1; i--) {
    fromLeftToRight(50 * i);
    ledOff(10);
    fromRightToLeft(50 * i);
    ledOff(10);
  }
  for (int i = 3; i >= 1; i--) {
    fromLeftToRight(50 );
    ledOff(10);
    fromRightToLeft(50 );
    ledOff(10);
  }
  ledOff(100);
  fromMiddleS(150);
  ledOff(100);
  fromBothSideS(100);
  ledOff(100);
  for (int i = 3; i >= 1; i--) {
    cross(50 * i);
  }
  for (int i = 3; i >= 1; i--) {
    cross(50);
  }
}

//  LED 灯全亮
void ledOn(int lightTime) {
  for (int i = 2; i <= 8; i ++ ) {
    digitalWrite(i, HIGH);
  }
  delay(lightTime);
}
//  LED 灯全灭
void ledOff(int lightTime) {
  for (int i = 2; i <= 8; i ++ ) {
    digitalWrite(i, LOW);
  }
  delay(lightTime);
}
//  LED灯从左到右
void fromLeftToRight(int lightTime) {
  for (int i = 2; i <= 8; i ++ ) {
    digitalWrite(i, HIGH);
    delay(lightTime);
  }
}
//  LED灯从左到右单个点亮
void fromLeftToRightS(int lightTime) {
  for (int i = 2; i <= 8; i ++ ) {
    digitalWrite(i, HIGH);
    delay(lightTime);
    digitalWrite(i, LOW);
  }
}
//  LED灯从右到左
void fromRightToLeft(int lightTime) {
  for (int i = 8; i >= 2; i -- ) {
    digitalWrite(i, HIGH);
    delay(lightTime);
  }
}
//  LED灯从右到左
void fromRightToLeftS(int lightTime) {
  for (int i = 8; i >= 2; i -- ) {
    digitalWrite(i, HIGH);
    delay(lightTime);
    digitalWrite(i, LOW);
  }
}
//  LED灯从中间向两端
void fromMiddle(int lightTime) {
  for (int i = 5; i >= 2; i--) {
    digitalWrite(i, HIGH);
    digitalWrite(10 - i, HIGH);
    delay(lightTime);
  }
}
//  LED灯从中间向两端单个点亮
void fromMiddleS(int lightTime) {
  for (int i = 5; i >= 2; i--) {
    digitalWrite(i, HIGH);
    digitalWrite(10 - i, HIGH);
    delay(lightTime);
    digitalWrite(i, LOW);
    digitalWrite(10 - i, LOW);
  }
}
//  LED灯从两端向中间
void fromBothSide(int lightTime) {
  for (int i = 2; i <= 5; i++) {
    digitalWrite(i, HIGH);
    digitalWrite(10 - i, HIGH);
    delay(lightTime);
  }
}
//  LED灯从两端向中间单个点亮
void fromBothSideS(int lightTime) {
  for (int i = 2; i <= 5; i++) {
    digitalWrite(i, HIGH);
    digitalWrite(10 - i, HIGH);
    delay(lightTime);
    digitalWrite(i, LOW);
    digitalWrite(10 - i, LOW);
  }
}
//  LED灯从两端交叉点亮
void cross(int lightTime) {
  for (int i = 2; i <= 8; i++) {
    digitalWrite(i, HIGH);
    digitalWrite(10 - i, HIGH);
    delay(lightTime);
    digitalWrite(i, LOW);
    digitalWrite(10 - i, LOW);
  }
}

