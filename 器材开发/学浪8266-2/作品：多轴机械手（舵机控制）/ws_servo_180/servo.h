#include "ESP8266_ISR_Servo.h"

#define MIN_MICROS      500   //0度脉冲
#define MAX_MICROS      2500  //180度脉冲

void servo_setup(){
  for(int i=0;i<pin_data.servo_count;i++){
    pin_data.servo_index[i] = ISR_Servo.setupServo(pin_data.dname[pin_data.servo_pins_index[i]], MIN_MICROS, MAX_MICROS);
    ISR_Servo.setPosition(pin_data.servo_index[i], 0);
  }
}

void servo_write(uint8_t index, uint16_t pos){
  ISR_Servo.setPosition(pin_data.servo_index[index], pos);
}
