
#include "PCA9685.h"



//使用pca9685控制版后，舵机的角度范围（MIN为0度位置，MAX为180度位置）
#define MIN_US 144
#define MAX_US 880

PCA9685 pwmController;

void servo_setup() {
  Wire.begin();
  pwmController.resetDevices();
  pwmController.init();
  pwmController.setPWMFrequency(100);
}

void servo_write(uint8_t index, uint16_t pos) {
  uint16_t us = map( pos, 0, 180, MIN_US, MAX_US );
  pwmController.setChannelPWM(index, us);
  Serial.print("get:");
  Serial.println(pwmController.getChannelPWM(0));
}

//#include "ESP8266_ISR_Servo.h"

//#define MIN_MICROS      500   //0度脉冲
//#define MAX_MICROS      2500  //180度脉冲

//void servo_setup(){
//  for(int i=0;i<pin_data.servo_count;i++){
//    pin_data.servo_index[i] = ISR_Servo.setupServo(pin_data.dname[pin_data.servo_pins_index[i]], MIN_MICROS, MAX_MICROS);
//    ISR_Servo.setPosition(pin_data.servo_index[i], 0);
//  }
//}
//
//void servo_write(uint8_t index, uint16_t pos){
//  ISR_Servo.setPosition(pin_data.servo_index[index], pos);
//}
