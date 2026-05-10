const int leftPin1 = 7; //BIN1连接引脚7
const int leftPin2 = 4; //BIN2连接引脚8
const int rightPin1 = 8; //AIN1连接引脚8
const int rightPin2 = 9; //AIN2连接引脚9
const int leftSpeed = 5; //PWB连接引脚5
const int rightSpeed = 6; //PWA连接引脚6
const int speedPWM = 150; //设置小车运行的初始速度

const int trigPin = 11;   // 定义超声波Trig引脚连接到D2引脚
const int echoPin = 10; // 定义超声波Echo引脚连接到D3引脚
const int minDist = 20;
const int maxDist = 40;
const int followDist = 30;
int distance;


void setup() {
  pinMode(leftPin1, OUTPUT);
  pinMode(leftPin2, OUTPUT);
  pinMode(rightPin1, OUTPUT);
  pinMode(rightPin2, OUTPUT);
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);  //设置D2引脚为输出模式
  pinMode(echoPin, INPUT);  //设置D3引脚为输入模式
}

void loop() {
  Serial.println(distance);
  followDrive();
}

//******************************
//  功能：跟随模式
//  参数：无
//*******************************
void  followDrive() {  //
  getDistance();  // 获取当前的距离
  if ((distance >= minDist) && (distance <= maxDist)) {
    analogWrite(leftSpeed, speedPWM);
    analogWrite(rightSpeed, speedPWM);
    if (distance > followDist) {  //
      forward();
    }
    else if (distance < followDist) {
      backward();
    }
    else {
      pause();
    }
  }
  else {
    pause();
  }

}
//******************************
//  功能：获取距离  参考迎宾机器人项目五
//  参数：无
//*******************************
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
