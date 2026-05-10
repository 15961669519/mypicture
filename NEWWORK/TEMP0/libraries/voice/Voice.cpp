
#if ARDUINO >=100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Voice.h"

Voice::Voice(int Rst,int Data, int Busy)
{
	_rst=Rst;
	_data=Data;
	_busy=Busy;
}

void Voice::VoiceNum(int num){
	  int tmp,val;
    int hundred=0;
    val=num;
    
    tmp=val/100;

    if (tmp>0){
        VoiceWord(35+tmp);
        VoiceWord(34);
        hundred=1;
    }
    val=val%100;
    tmp=val/10;
    if (tmp>0){
        VoiceWord(35+tmp);
        VoiceWord(35);
        hundred=1;
    }
    else if(hundred==1){
        VoiceWord(32);
    }
    val=val%10;
        Serial.println(val);
    
    if (val>0){
        VoiceWord(35+val);
    }
}

void Voice::VoiceWord(int  num){
  digitalWrite(_rst,HIGH);
  delayMicroseconds(100);
  digitalWrite(_rst,LOW);
  delayMicroseconds(100);
  
  for(char i=0;i < num;i++)
  {   
        digitalWrite(_data,HIGH);             //数据脉冲高
        delayMicroseconds(100);         //延时100US
        digitalWrite(_data,LOW);            //数据脉冲低
        delayMicroseconds(100);        //延时100US
   } 

  while(digitalRead(_busy));  
}