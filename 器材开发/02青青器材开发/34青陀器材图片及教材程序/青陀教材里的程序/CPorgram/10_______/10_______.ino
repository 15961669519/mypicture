const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
const int intSpeedPWM = 150; //设置小车运行的初始速度

void setup() {
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int delayTime = 2000;
  analogWrite(leftSpeed, intSpeedPWM); //❷设定左侧电机的速度
  analogWrite(rightSpeed, intSpeedPWM); //❷设定右侧电机的速度
  forward();      //前进
  delay(delayTime);
  backward();  //后退
  delay(delayTime);
  turnLeft();  //左转
  delay(delayTime);
  turnRight();  //右转
  delay(delayTime);
  rotateLeft();  //原地左转
  delay(delayTime);
  rotateRight();  //原地右转
  delay(delayTime);
  pause();  //停止
  delay(delayTime);
}

void getDistance() {
  digitalWrite(trigPin, LOW);   //Trig引脚拉低，准备生成触发脉冲
  delayMicroseconds(2);        //延时2ms
  digitalWrite(trigPin, HIGH);  //Trig引脚拉高
  delayMicroseconds(10);     //延时10ms
  digitalWrite(trigPin, LOW);  //Trig引脚拉低，生成触发脉冲完毕
  distance = pulseIn(echoPin, HIGH) / 58;  //获得距离

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
