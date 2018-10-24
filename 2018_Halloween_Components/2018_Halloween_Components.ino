#include <Button.h>

// LEDs
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 3
#define MODE_BLUE 0
#define MODE_RED 1
int current_mode = MODE_BLUE;

// SENSORS
#define POT_PIN 0
#define BUTTON_PIN 2
const int DEBOUNCE = 50;
Button button(BUTTON_PIN, DEBOUNCE);
int brightness;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  button.begin();
  
  Serial.begin(9600);
}

void loop() {
  
  // change mode on button press
  if ( button.pressed() ) {
    toggleMode();
  }
  
  // read potentiometer value
  int pot_value = analogRead(POT_PIN);
  
  // set LED brightness
  brightness = map(pot_value, 0, 1023, 0, 255);

  // write to LED
  if ( current_mode == MODE_BLUE ) {
    ledBlue(); 
  } else {
    ledRed();
  }
}

void ledRed() {
  analogWrite(RED_PIN, brightness);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
}

void ledBlue() {
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, brightness);  
}

void toggleMode() {
  current_mode = ( current_mode == MODE_BLUE ) ? MODE_RED : MODE_BLUE;
}
