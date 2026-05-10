/*
  AnalogReadSerial

  Reads an analog input on pin 0, prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogReadSerial
*/

// the setup routine runs once when you press reset:
void setup() {
  // 定义波特率
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
  // 从A0针脚读取模拟量的数字
  int sensorValue = analogRead(A0);
  // 发送到串口监视器
  Serial.println(sensorValue);
  delay(1);        // 读取的间隔时间
}
