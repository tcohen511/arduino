

// ————————————————————————————————————————————————

/* 
 * CLASS DECLARATIONS 
 */

class AnalogButton {
      
  #define CURRENT 0
  #define PREVIOUS 1
  #define CHANGED 2
      
      // Class member variables
        byte pin ;
        int threshold ;
        unsigned long debounce ;
      
      // State variables
        unsigned long lastDebounceTime ;
        byte reading ;
        byte state ;
  
  // Constructor  
  public: 
  AnalogButton(byte buttonPin, int buttonThreshold, unsigned long debounceTime = 50) {
    pin = buttonPin ;
    threshold = buttonThreshold ;
    debounce = debounceTime ;
    state = 0 ;
    bitWrite(state,CURRENT,1) ;
    reading = 0 ;
    bitWrite(reading,CURRENT,1) ;
    lastDebounceTime = 0 ;
  }
  
  /*
   * Adjust threshold value for button.
   */
  void AnalogButton::adjustThreshold(int newThreshold) {
    threshold = newThreshold ;
   }
  
  
  /*
   * Adjust debounce time. Leave blank to return to default.
   */
  void AnalogButton::adjustDebounce(unsigned long newDebounce) {
    debounce = newDebounce ;
   }
  
  
  /*
   * Update previous and current states, update whether state 
   * has changed since last reading, and return current state.
   */
  
  bool AnalogButton::isPressed() {
  // update variables
    bitWrite(reading,PREVIOUS,bitRead(reading,CURRENT)) ;
    bitWrite(state,PREVIOUS,bitRead(state,CURRENT)) ;
  
    /* Test whether reading has changed since last time; 
     * if so, reset debounce time.
     */
    bitWrite(reading,CURRENT, (analogRead(pin) > threshold)) ;
    if (bitRead(reading,CURRENT) != bitRead(reading,PREVIOUS)) {
      lastDebounceTime = millis() ;
    }
    
    /* Test whether reading has been consistent for longer 
     * than debounce time; if so, update current state.
     * 
     */
    if ((millis() - lastDebounceTime >= debounce)) {
      bitWrite(state,CURRENT, bitRead(reading,CURRENT)) ;
    }
  
    /* If current state doesn't equal previous state, 
     * update CHANGED byte to true. 
     */
    bitWrite(state,CHANGED, 
             bitRead(state,CURRENT) != bitRead(state,PREVIOUS)) ;
  
    return bitRead(state,CURRENT) ;
   }
  
  
  /*
  * Return true if button was pressed at last reading.
  * (Does not take new reading.)
  */
   bool AnalogButton::wasPressed() {
    if (bitRead(state,CURRENT)) {
      return true;
    } else {
      return false;
    }
   }
  
  
  /*
   * Return true if state has been changed.
   */
   bool AnalogButton::stateChanged() {
    if (bitRead(state,CHANGED)) {
      return true;
    } else {
      return false;
    }
   }
  
  
  /*
   * Return true if button is pressed and was not pressed before.
   * (Takes new reading.)
   */
   bool AnalogButton::newPress() {
    return (isPressed() && stateChanged());
   }
  
  
  } ; // end class declaration


// ————————————————————————————————————————————————

/* 
 * INPUTS
 */

// SENSORS

// Calibration:
const int analogThreshold = 2 ; // voltage at which to register tap
unsigned long debounceDelay = 60 ; // debouncing parameter

// Button Inputs:
const byte recordingButtons = 3 ; // number of index buttons
const byte buttonPins[] = {A11,A7,A9,A10} ; // index button pins in order, then capture button pin


// INDEX VALUE INPUTS
const byte recordingMaxDigits = 3 ; // number of digits in index value system


// LIGHTS

int ledPin = 3 ;
int minBrightness = 10 ; 
int maxBrightness = 255 ;
int fadeAmount = 5 ;
int fadeStep = 30 ; // in ms

int brightness = 0 ;
unsigned long lastStepTime = 0 ;




// ————————————————————————————————————————————————

/* 
 * CALCULATIONS / INSTANTIATIONS
 */

const byte captureButtonIndex = recordingButtons ;
const byte numButtons = recordingButtons + 1 ; // total number of buttons


// Instantiate buttons
AnalogButton buttons[numButtons] = {
  AnalogButton(buttonPins[0], analogThreshold, debounceDelay),
  AnalogButton(buttonPins[1], analogThreshold, debounceDelay),
  AnalogButton(buttonPins[2], analogThreshold, debounceDelay),
  AnalogButton(buttonPins[3], analogThreshold, debounceDelay),
} ;

/*
AnalogButton *buttons[numButtons];

void createButtons() {

  for (int i = 0; i<numButtons; i++) {
    buttons[i] = new AnalogButton(buttonPins[i], analogThreshold, debounceDelay) ;
  } 

}
*/

byte recordingDigitIndex = 0 ; // the index level of the next recorded digit (0=1000s,1=100s)
byte recordedTaps = 0 ;
long trackIndex ; // index value to be sent to RasPi, to access recorded track


// ————————————————————————————————————————————————

/* 
 * LOOPS
 */

void setup() {
  pinMode(ledPin, OUTPUT) ;
  Serial.begin(9600) ;
  //createButtons();
}


void loop() {
  // Capture index values from the buttons and, when it gets
  // long enough, send index number to Raspberry Pi

  // Capture button presses and record in array
  if (recordingDigitIndex < recordingMaxDigits) {
    recordTaps();
  } 
  else { // calculate index number, send to RP and reset values
    sendIndexNumber() 
  }


  
  sendButtonPresses();

  lightFade() ;
}


// ————————————————————————————————————————————————

/* 
 * FUNCTIONS
 */

// SENSOR FUNCTIONS

// send button presses to Raspberry Pi
void sendButtonPresses() {
  for (int i=0; i<numButtons; i++) {
    if (buttons[i].newPress()) {
      Serial.print(String(i+1)) ;
    }
  }
}

// if (buttons[i]->newPress()) {

// Capture button presses and record in array
void recordTaps() {
  for (int i=0; i<recordingButtons; i++) {
    if (buttons[i].newPress()) {
      bitWrite(recordedTaps, recordingDigitIndex, i+1) ;
      recordingDigitIndex++ ;
    }
  }
}

void sendIndexNumber() {
    for(int i=0; i < recordingDigitIndex; i++) {
      trackIndex = trackIndex + bitRead(recordedTaps, i) * pow(10, recordingMaxDigits-i-1);
    } 
    String arduinoOuput = String(trackIndex) ;
    Serial.println(trackIndex) ;

    recordedTaps = 0 ;
    recordingDigitIndex = 0 ;
}

// LIGHT FUNCTIONS

void lightFade() {
  if (millis() - lastStepTime > fadeStep) {
    lastStepTime = millis() ;
  
    // set the brightness of ledPin:
    analogWrite(ledPin, brightness);
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= minBrightness || brightness >= maxBrightness) {
      fadeAmount = -fadeAmount;
    }  
  }
}


// ————————————————————————————————————————————————



