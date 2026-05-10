/**
   从本质上讲，这些传感器在部件之间有很大的差异和漂移，并且电阻的“绝对”值随湿度/温度等而变化，
   因此将传感器置于干净，低eCO2 / TVOC环境中，让它悬挂半小时，然后读取基线。
   这将提供洁净室的“基线”读数。
   然后保存该值并将其重新存储在仅针对该传感器的启动上，此时传感器可以比较其“清洁空气”时的电阻。
*/
#include "Adafruit_SGP30.h"
Adafruit_SGP30 sgp;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}


void sgp30_setup() {

  Serial.println("SGP30 start");


  Wire.begin(IIC_SDA, IIC_SCL);

  if (! sgp.begin()) {
    Serial.println("Sensor not found :(");
    while (1);
  }

  //手动矫正SGP30的精度
  //sgp.setIAQBaseline(0x8C35, 0x9182); 

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}
int sgp_counter = 0;
void sgp30() {

  // 可配合温湿度传感器，进行精度矫正（比如sht30）
  sgp.setHumidity(getAbsoluteHumidity(sht_data.temperature, sht_data.humidity));

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  sgp_counter++;
  if (sgp_counter == 30) {
    sgp_counter = 0;
    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }

    Serial.print("****Baseline values: eCO2: 0x");
    Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
  //  if (! sgp.IAQmeasureRaw()) {
  //    Serial.println("Raw Measurement failed");
  //    return;
  //  }
  //  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  //  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

}
