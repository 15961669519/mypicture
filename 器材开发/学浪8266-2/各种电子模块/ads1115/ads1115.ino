#include "ADS1X15.h"

#define ESP8266

ADS1115 ADS(0x48);

void setup() {
  Serial.begin(115200);
  ADS.begin(D1,D2);
//  ADS.readADC(3);
}

void loop()
{
  ADS.setGain(0);

  int16_t val_0 = ADS.readADC(0);  
  int16_t val_1 = ADS.readADC(1);  
  int16_t val_2 = ADS.readADC(2);  
  int16_t val_3 = ADS.readADC(3);  
  float f = ADS.toVoltage(1);  // voltage factor
  Serial.println(ADS.getMaxVoltage());

  Serial.print("\tAnalog0: "); Serial.print(val_0); Serial.print('\t'); Serial.println(val_0 * f, 3);
  Serial.print("\tAnalog1: "); Serial.print(val_1); Serial.print('\t'); Serial.println(val_1 * f, 3);
  Serial.print("\tAnalog2: "); Serial.print(val_2); Serial.print('\t'); Serial.println(val_2 * f, 3);
  Serial.print("\tAnalog3: "); Serial.print(val_3); Serial.print('\t'); Serial.println(val_3 * f, 3);
  Serial.println();

  delay(1000);
}
