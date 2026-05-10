void setup() {

  Serial.begin(9600);
  Serial.println(" -- 等差数列 -- ");
  for (int i = 1; i <= 5; i++) {
    Serial.print(i);
    Serial.print('\t');
  }
  Serial.println();
  Serial.println(" -- 三角数列 -- ");
  for (int i = 5; i >= 1; i--) {
    for (int j = i; j >= 1; j--) {
      Serial.print(j);
      Serial.print('\t');
    }
    Serial.println();
  }
}

void loop() {
}


