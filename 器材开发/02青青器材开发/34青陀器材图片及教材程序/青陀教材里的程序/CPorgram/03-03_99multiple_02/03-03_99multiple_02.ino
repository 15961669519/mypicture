void setup() {
  Serial.begin(9600);
  for (int i = 1; i <= 9; i ++) {
    for (int j = 1; j <= i; j ++) {
      Serial.print(j);
      Serial.print("*");
      Serial.print(i);
      Serial.print("=");
      Serial.print((i * j));
      Serial.print('\t');
    }
    Serial.println();
  }
}

void loop() {

}
