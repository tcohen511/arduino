const int sensorPins[3] = {A11,A7,A10} ;
const int inputs = 3 ;

int pin1 = 0 ;
int pin2 = 0 ;

void setup() {
  Serial.begin(9600) ;
}

void loop() {
  pin1 = analogRead(sensorPins[0]) ;
  pin2 = analogRead(sensorPins[1]) ;
  Serial.println(pin1) ;
  Serial.println(pin2) ;
  delay(10) ;
  /*
  for (int i=0; i<inputs; i++) {
    Serial.println(analogRead(sensorPins[i])) ;
    delay(10) ;
  }
  */
}
