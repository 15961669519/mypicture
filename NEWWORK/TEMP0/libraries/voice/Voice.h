#ifndef Voice_H
#define Voice_H
#if defined (ARDUINO) && ARDUINO>=100
	#include "Arduino.h"
#else
	#include "Wprogram.h"
#endif

class Voice{

public:
	
	Voice(int Rst,int Data, int Busy);
	void VoiceWord(int Sequence);
	void VoiceNum(int num);

private:
	int _rst;   // Pin 6
	int _data;	// pin 7
	int _busy;	// pin 8
};
#endif

