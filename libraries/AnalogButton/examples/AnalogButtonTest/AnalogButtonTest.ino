#include <AnalogButton.h>

#define BUTTON_PIN 10
#define THRESHOLD 0.25 // voltage at which to register tap
#define DEBOUNCE 30 // ms

AnalogButton button = AnalogButton(BUTTON_PIN, THRESHOLD, DEBOUNCE);

void setup() {
  Serial.begin(9600) ;
}

void loop() {
  
  if ( button.newPress() ) {
    Serial.println("tap");
  }
}