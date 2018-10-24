

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

// MODES (set initiation sequences)
long recordModeIndex = 11 ;
long passiveLightModeIndex = 22 ;
long activeLightModeIndex = 444 ;
long offModeIndex = 33 ; // turn off

const int numModes = 4;


// SENSORS

// Calibration:
const int analogThreshold = 2 ; // voltage at which to register tap
unsigned long debounceDelay = 60 ; // debouncing parameter

// Button Inputs:
const byte recordingButtons = 3 ; // number of index buttons
const byte buttonPins[] = {A11,A7,A10} ; // index button pins in order, then capture button pin


// INDEX VALUE INPUTS
const byte recordingMaxDigits = 2 ; // number of digits in index value system

// LIGHTS

int ledPin = 3 ;
int minBrightness = 10 ; 
int maxBrightness = 255 ;
int fadeAmount = 5 ;
int fadeStepTime = 15 ; // in ms

int brightness = 0 ;
unsigned long lastStepTime = 0 ;




// ————————————————————————————————————————————————

/* 
 * CALCULATIONS / INSTANTIATIONS
 */

//const byte captureButtonIndex = recordingButtons ;
const byte numButtons = recordingButtons + 0 ; // total number of buttons

// Instantiate buttons
AnalogButton buttons[numButtons] = {
  AnalogButton(buttonPins[0], analogThreshold, debounceDelay),
  AnalogButton(buttonPins[1], analogThreshold, debounceDelay),
  AnalogButton(buttonPins[2], analogThreshold, debounceDelay),
} ;

// Set recording parameters
byte recordingDigitIndex = 0 ; // the index level of the next recorded digit (0=1000s,1=100s)
byte recordedTaps[recordingMaxDigits] ;
int indexNum = 1 ; // index value to be sent to RasPi, to access recorded track

// Define modes and store
#define offMode 0
#define recordMode 1
#define passiveLightMode 2
#define activeLightMode 3


byte modes[numModes] = {offMode,recordMode,passiveLightMode,activeLightMode} ;
long modeIndices[numModes] = 
    {offModeIndex, recordModeIndex, passiveLightModeIndex,activeLightModeIndex} ;

byte currentMode = 0 ;
bool modeChange = 0 ;


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

  // Listen for mode initializer
  recordTaps() ;
  if (getIndexNum()>0) {
  sendIndexNum() ;
  }


/*
  Serial.print("index num: ");
  Serial.println(indexNum) ;
  Serial.print("recording digit index: ");
  Serial.println(recordingDigitIndex) ;
  */

  setMode() ;
/*
  if (modeChange) {
    Serial.print("New mode: ") ; ///////////
    Serial.println(currentMode) ; ///////////
  }
 */

switch (currentMode) {
  
// RECORDING MODE
  case recordMode:
    
    lightFade() ;
    
    if (!(recordingDigitIndex < recordingMaxDigits)) {
      getIndexNum() ;
      if (!setMode()) { // If input isn't a mode initializer
        sendIndexNum() ;
        resetRecordingVars() ;
      }
    }
  }

// PASSIVE LIGHT MODE
  else if (currentMode == passiveLightMode) {
    lightFade() ;
  }

// ACTIVE LIGHT MODE
  else if (currentMode == activeLightMode) {
    lightOff() ;
  }
}


// ————————————————————————————————————————————————

/* 
 * FUNCTIONS
 */

// SENSOR FUNCTIONS

// Check whether index value is a mode code. 
// If so, sets the corresponding mode and returns true. 
bool setMode() {
  for (int i=0; i<numModes; i++) {
    if (indexNum == modeIndices[i]) {
      Serial.println(indexNum) ;
      if (currentMode != modes[i]) {
        currentMode = modes[i] ;
        lightOff() ;
        modeChange = true ;
      }
      else {
        modeChange = false ;
      }
    resetRecordingVars() ;
    return true;  
    }
  }
  modeChange = false ;
  return false;
}


  /*
  if (!(isModeCode)) {
    modeChange = false ;
  }

  
  }
  if (indexNum == listeningModeIndex) {
    if (currentMode != listeningMode) {
      currentMode = listeningMode ;
      lightOff() ;
      modeChange = true ;
      //Serial.println("Listening Mode") ; /////
    }
    else {
      modeChange = false ;
    }
    resetRecordingVars() ;
    return true ;
  } 
  else if (indexNum == recordModeIndex) {
    if (currentMode != recordMode) {
      currentMode = recordMode ;
      modeChange = true ;
      //Serial.println("Record Mode") ; /////
    }
    else {
      currentMode = recordMode ;
      modeChange = false ;
    }
    resetRecordingVars() ;
    return true ;
  } 
  else if (indexNum == passiveLightModeIndex) {
    if (currentMode != passiveLightMode) {
      currentMode = passiveLightMode ;
      modeChange = true ;
      //Serial.println("Passive Light Mode") ; /////
    }
    else {
      modeChange = false ;
    }
    resetRecordingVars() ;
    return true ;
  }
  else if (indexNum == activeLightModeIndex) {
    if (currentMode != activeLightMode) {
      currentMode = activeLightMode ;
      modeChange = true ;
      //Serial.println("Active Light Mode") ; /////
    }
    else {
      modeChange = false ;
    }
    resetRecordingVars() ;
    return true ;
  }
  else {
    modeChange = false ;
    return false ;
  }
}
*/



// send button presses to Raspberry Pi
void sendButtonPresses() {
  for (int i=0; i<numButtons; i++) {
    if (buttons[i].newPress()) {
      Serial.print(String(i+1)) ;
    }
  }
}


// Capture button presses and record in array
// If max digits already record, push everything back
// and record new digit.

void recordTaps() {
  for (int i=0; i<recordingButtons; i++) {
    if (buttons[i].newPress()) {
      //Serial.print("pin: "); //
      //Serial.println(i+1); ///////
      if (recordingDigitIndex < recordingMaxDigits) {
        recordedTaps[recordingDigitIndex] = i+1 ;
        recordingDigitIndex++ ;  
      }
      else {
        for (int j=0; j<recordingMaxDigits-1; j++) {
          recordedTaps[j] = recordedTaps[j+1] ;
        }
        recordedTaps[recordingMaxDigits-1] = i+1 ;
      }    
    }
  }
}

// Calculate index number, send to RP and reset values
// Iterates over recordedTaps int [4]
// [1, 2, 3, 4] 
int getIndexNum() {
  indexNum = 0;
  for(int i=0; i < recordingDigitIndex; i++) {
    indexNum = indexNum * 10 + recordedTaps[i];
    // indexNum = indexNum + round(recordedTaps[i]) * pow(10, recordingMaxDigits-i-1);
  } 
  return indexNum;
}

void sendIndexNum() {
  String arduinoOuput = String(indexNum) ;
  Serial.println(indexNum) ;
}

void resetRecordingVars() {
  for (int i=0; i<recordingMaxDigits; i++) {
    recordedTaps[i] = 0 ; 
  }
  recordingDigitIndex = 0 ;
}

// LIGHT FUNCTIONS

void lightFade() {
  if (brightness == 0) {
    brightness = minBrightness ;
  }
  if (millis() - lastStepTime > fadeStepTime) {
    lastStepTime = millis() ;
  
    // set the brightness of ledPin:
    analogWrite(ledPin, brightness);
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= minBrightness || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }  
  }
}

void lightOff() {
  analogWrite(ledPin, 0) ;
}


// ————————————————————————————————————————————————



