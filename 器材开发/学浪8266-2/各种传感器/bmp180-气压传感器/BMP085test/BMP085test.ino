#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

void setup() {
  
  Serial.begin(115200);
  
  //初始化传感器，如果初始化失败，会自动重启
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
}

void loop() {
  // 输出温度数据(可作为温度计，此处主要用于气压计算的温度补偿)
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
  
  // 输出气压(Pa，换算hPa要除以100)
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  // 根据气压计算高度
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  //如果你知道当前真实的高度，可以计算出海平面气压
  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(bmp.readSealevelPressure());
  Serial.println(" Pa");

  // 如果你知道真实的海平面压力，可以手动计算当前的高度
  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(101500));
  Serial.println(" meters");

  Serial.println();
  delay(500);
}
