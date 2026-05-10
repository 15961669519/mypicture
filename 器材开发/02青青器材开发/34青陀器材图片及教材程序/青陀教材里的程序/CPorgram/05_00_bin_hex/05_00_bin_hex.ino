void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Dec   Bin     Oct     Hex");
  for (int i = 0; i <= 15; i++) {
    Serial.print(i);
    Serial.print('\t');
    Serial.print(i, BIN);
    Serial.print('\t');
    Serial.print(i, OCT);
    Serial.print('\t');
    Serial.print(i, HEX);
    Serial.println();
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
