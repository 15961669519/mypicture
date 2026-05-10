const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
const int speedPWM = 120; //设置小车运行的初始速度

const int ldrLeft = A2;  // 左侧光敏电阻连接引脚
const int ldrRight = A1; // 右侧光敏电阻连接引脚
const int ldrBack = A3;  // 后方光敏电阻连接引脚
const int scope = 10;  // 左右光敏电阻的容许的偏差

int rangeLeft = 1023, rangeRight = 1023, rangeBack = 1023; // 记录光敏电阻的标定值
int leftValue, rightValue, backValue;  // 三个光敏电阻的返回值


void setup() {
  Serial.begin(9600);
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  for (int i = 0; i <= 200; i++) {
    leftValue = analogRead(ldrLeft);
    rightValue = analogRead(ldrRight);
    backValue = analogRead(ldrBack);
    rangeLeft = (leftValue < rangeLeft) ? leftValue : rangeLeft;
    rangeRight = (rightValue < rangeRight) ? rightValue : rangeRight;
    rangeBack = (backValue < rangeBack) ? backValue : rangeBack;
    delay(50);
  }
  digitalWrite(13, LOW);

  Serial.println(String() + rangeLeft + " -- " + rangeRight + " -- " + rangeBack);
  int minValue;
  minValue = min(rangeLeft, rangeRight);
  minValue = min(minValue, rangeBack);
  rangeLeft -= minValue;
  rangeRight -= minValue;
  rangeBack -= minValue;
  Serial.println(String() + rangeLeft + " -- " + rangeRight + " -- " + rangeBack);

}

void loop() {
  lightTrack();
}
//******************************
//  功能：巡光模式
//  参数：无
//*******************************
void lightTrack() {
  int carDirect;
  analogWrite(leftSpeed, speedPWM); //❷设定左侧电机的速度
  analogWrite(rightSpeed, speedPWM); //❷设定右侧电机的速度
  // 分别读取光敏电阻的值，减去各自的标定值
  leftValue = analogRead(ldrLeft) - rangeLeft;
  rightValue = analogRead(ldrRight) - rangeRight;
  backValue = analogRead(ldrBack) - rangeBack;
  Serial.println(String() + leftValue + " -- " + rightValue + " -- " + backValue);
  //  delay(100);

  carDirect = min(leftValue, rightValue); // 取左右两侧光敏电阻最小值
  if (carDirect < backValue) {   // 小于后侧，表明小车迎着光源。
    if (abs(leftValue - rightValue) < scope) {  // 左右两侧的差值小于设定的容许值，光源在前方
      forward();   // 前进
    }
    else if (leftValue > rightValue) {    // 光源在右侧
      turnRight();  // 右转
      delay(10);
    }
    else {
      turnLeft();  // 左转
      delay(10);
    }
  }
  else {  // 小车背对光源
    rotateLeft();   // 原地左转
    delay(10);
  }
}

//===============
// 前进
//===============
void forward() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 后退
//===============
void backward() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 1);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 1);
}
//===============
// 左转
//===============
void turnLeft() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 右转
//===============
void turnRight() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 0);
}
//===============
// 原地左转
//===============
void rotateLeft() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 1);
  digitalWrite(rightPin1, 1);
  digitalWrite(rightPin2, 0);
}
//===============
// 原地右转
//===============
void rotateRight() {
  digitalWrite(leftPin1, 1);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 1);
}
//===============
// 停止
//===============
void pause() {
  digitalWrite(leftPin1, 0);
  digitalWrite(leftPin2, 0);
  digitalWrite(rightPin1, 0);
  digitalWrite(rightPin2, 0);
}
