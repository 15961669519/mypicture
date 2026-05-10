#include <EEPROM.h>
#include <arduino.h>
#define EEPROM_SIZE 72

void Flash_write( const uint8_t *data,  uint8_t size)
{
unsigned char i;
int addr = 0;

if (!EEPROM.begin(EEPROM_SIZE))
{
  Serial.println("failed to initialise EEPROM"); delay(1000000);
}

for(i=0;i<size;i++){

EEPROM.write(addr+i, data[i]);

}
 EEPROM.commit();
}





 void Flash_read( uint8_t *data,  uint8_t size)
{
		
		const int addr = 0;

		if (!EEPROM.begin(EEPROM_SIZE))
		{
		  Serial.println("failed to initialise EEPROM"); delay(1000000);
		}
   
		for(int i=0;i<size;i++){

	  data[i]=byte(EEPROM.read(i));
	  

		}
}

