const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
 int speedPWM = 80; //设置小车运行的初始速度
const int leftTrackPin = 3;
const int rightTrackPin =2;


void setup() {
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);
  pinMode(leftTrackPin, INPUT);
  pinMode(rightTrackPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  lineTrack();
}
//******************************
//  功能：循迹模式
//  参数：无
//*******************************
void lineTrack() {
  int leftValue = digitalRead(leftTrackPin); //读取左侧引脚值
  int rightValue = digitalRead(rightTrackPin); //读取右侧引脚值

  Serial.print(leftValue); //输出左侧循迹模块的值到串口监视器
  Serial.print("--");
  Serial.println(rightValue);//输出右侧循迹模块的值到串口监视器
  analogWrite(leftSpeed, speedPWM); //设定左侧马达的速度
  analogWrite(rightSpeed, speedPWM); //设定右侧马达的速度

  if ((leftValue == 1) && (rightValue == 1)) { //正常
    forward();
  } else if  ((leftValue == 0) && (rightValue == 1)) { //右偏
//    while (!digitalRead(leftTrackPin)) {
      rotateLeft();
      delay(5);
//    }
  } else if ((leftValue == 1) && (rightValue == 0)) { //左偏
//    while (!digitalRead(rightTrackPin)) {
      rotateRight();
      delay(5);
//    }
  } else if  ((leftValue == 0) && (rightValue == 0)) { //双偏
    pause();
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
