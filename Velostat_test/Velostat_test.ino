// Velostat test

const int inputPin = 0;

void setup() {
  Serial.begin(9600);

}

void loop() {
  Serial.println(analogRead(inputPin));
  delay(100);
}
