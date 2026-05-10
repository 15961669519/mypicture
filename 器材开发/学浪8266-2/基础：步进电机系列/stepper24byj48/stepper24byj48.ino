//某网友写的适用于8266的步进电机驱动库
#include <Stepper2.h>

//控制步进电机的引脚
int pinOut[4] = { D1, D2, D3, D4 };

//实例化对象
Stepper2 myStepper(pinOut);

//开机函数
void setup() {
  Serial.begin(115200);
  myStepper.setSpeed(14); //28byj-48 每分钟最多14圈左右，继续增加无意义
}

void loop() {
  myStepper.setDirection(1); // 正转，改为0则是反转
  myStepper.turn(1);          //圈数
  myStepper.stop();
  delay(2000);

  //如需转动不足1圈，请使用step(脉冲数量)的方法
  //注意，4096个脉冲为一圈，但不能一次性设置，至少要分成8次循环，加delay(1)延迟
  
  for (int i = 0; i < 8; i++)
  {
    myStepper.step(4096 / 8);
    delay(1);
  }
}
