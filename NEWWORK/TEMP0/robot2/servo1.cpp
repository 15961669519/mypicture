#include "servo1.h"


#include "pwm.h"
#include <Arduino.h>

#include "option.h"


        const static int kFrequency = 120;
        const static int kTickUs = 2;
        const static int kServoMiddle = 1400;
        const static int kServoMax = 2000;
        const static int kServoMin = 800;
        const static int kServoRange = kServoMax-kServoMiddle;

		#ifdef PCA9685_PWM
        PCA9685 pwm1 = PCA9685(0x43);
		#endif
        
 		int channel=0;
		int freq=1000;
		int resolution=10;

void pwm_init(void){

		pwm1.begin();  
		pwm1.setPWMFreq(kFrequency);
  
}
    void Servo::init(void) {


		pwm1.begin();  
		pwm1.setPWMFreq(kFrequency);




    }
   void Servo::detach(void){

   	}
	bool Servo::attached(void){
	return 1;
		}
	void Servo::write(float position){
	setAngle(position); 
		}
    Servo::Servo(int Index, int partIndex) {
        pwmIndex_ =partIndex;
		//Serial.printf("pwmIndex_ is:%d,index is :%d \r\n",pwmIndex_,Index);
        //inverse_ = partIndex == 1 ? true : false;
        Index_=Index;
        adjust_angle_ = 0;//adjust_angle_ = partIndex == 1 ? 15.0 : 0.0;
        range_ = 0;//range_ = partIndex == 0 ? 45 : 60;
        angle_ = 0;
		scale_=0;
		inverse_=0;
		//offset_=0;
    }

    void Servo::setAngle(float angle) {

//		Serial.printf("exceed[%d][%f]\r\n", pwm2hexapod(pwmIndex_), angle);

        angle_ = angle;
        
		#ifdef PCA9685_PWM
		
    // init();
	 
        PCA9685* pwm;
        pwm = &pwm1;
		#endif
        
             int idx;
            idx = pwmIndex_;


        //Serial.printf("exceed[%d][%f]\r\n", pwm2hexapod(pwmIndex_), angle);
     
        angle -= adjust_angle_;
        //if (inverse_)
            //angle = -angle;
		//Serial.printf("exceed[%d][%f]\r\n", pwm2hexapod(pwmIndex_), angle);

        int us = kServoMiddle + (angle-90.0)*(kServoRange/60)*(1+0.01*scale_);
		//Serial.printf("kServoMiddle:%d + offset_:%d + angle:%f*(kServoRange:%d/60)*(1+0.01*scale_:%d)\r\n",kServoMiddle,offset_,angle,kServoRange,scale_);
        if (us > kServoMax)
            us = kServoMax;
        else if(us < kServoMin)
            us = kServoMin;
		
	 
	 //Serial.printf("idx:%d\r\n",idx);
	// Serial.printf("idx:%d,us/kTickUs:%d\r\n",idx,us/kTickUs);
		//pwm->setPWM(idx, 0, 1000);
		
        pwm->setPWM(idx, 0, (us/kTickUs));
		
		
     // Serial.printf("pwmidx is %d ,setAngle(%.2f, %d)\r\n", idx,angle, us);
    }

    float Servo::getAngle(void) {
        return angle_;
    }

