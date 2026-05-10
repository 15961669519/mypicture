#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
int bmp_pa = 0;

void bmp180_setup() {
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
//    while (1) {}
  }
}

void bmp180() {
  bmp_pa = bmp.readPressure()/100;
}
