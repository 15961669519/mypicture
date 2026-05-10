/*
  ReadAnalogVoltage

  Reads an analog input on pin 0, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/ReadAnalogVoltage
*/

// the setup routine runs once when you press reset:
void setup() {
  // 定义波特率
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
  // 读取模拟量
  int sensorValue = analogRead(A0);
  //将模拟量转为电压（模拟量0-1023，对应的是电压0-3.3v）做个运算就好
  float voltage = sensorValue * (3.3 / 1023.0);
  // 输出到串口显示
  Serial.println(voltage);
}
