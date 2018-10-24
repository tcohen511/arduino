
/* 
 *  In this program, presses of buttons / vibration of piezos are captured,
 * translated into an index number, and the used to trigger an action 
 * associated with that index number.
 * 
 * Buttons that write index number are assigned an integer value [0, x-1].
 * The integer value of the pressed button determined the digit to be 
 * recorded, with initial digit recorded in the 10^0, then 10^1, etc.
 * A maximum place value of the recorded index is determined. When a digit
 * is recorded in that place value, the index value is 'captured' and the 
 * records are reset. 
 * 
 * The index value can also be captured before the maximum place value is 
 * populated by pressing the 'capture button', a separate button used solely 
 * for that purpose.
 * 
 * ———————
 * 
 * This is a test for a halloween costume idea that will do the same, using 
 * sensors on the fingertips of a glove and trigger pre-recorded speech 
 * segments to play through a bluetooth speaker, allowing user to have a 
 * converation without speaking.
 * 
 * (Final project with hopefully also have a trigger to initialize this
 * "fugue state", with perhaps LED lights and other sound effects to 
 * signal this to conversation partners.)
 * 
 * Further things to explore include queuing, cutting off a recording before 
 * it's been completed, 'quick-access' recordings that can be accessed with
 * single button press (would likely require second hand's worth of sensors), 
 * volume control, smoothing transitions between audio segments, etc. – 
 * generally, playback control.
 * 
 */

// set analog input threshold to register tap
const int analogThreshold = 0;

// set the number of 'recording' buttons
const int recordingButtons = 2;

// set index of non-recording buttons 
// within our button data arrays
const int captureButtonIndex = recordingButtons;
const int playButtonIndex = recordingButtons + 1; // not initialized, here as e.g.

const int buttons = recordingButtons + 2; // total number of buttons

// set input pin numbers of buttons
const int buttonPins[] = {3,2,0};

// array for capturing button states
int buttonStates[buttons];

// array for retaining previous button states
int buttonLastStates[buttons];

int recordingDigit; // the next digit to be recorded
int recordingDigitIndex = 0; // place value of next recorded digit
const int recordingMaxDigits = 5; // max number of digits in recording array
long recordingArray[recordingMaxDigits]; // where recorded digits are stored

long recordIndex; // the index value of the stored record to be accessed

// set debouncing parameters
unsigned long debounceDelay = 50; 

// —————————————————————————————————————————


void setup() {
  
  // initialize button input pins (only for digital inputs)
  for(int i=0; i < recordingButtons; i++) {
    pinMode(buttonPins[i], INPUT);
  }

  Serial.begin(9600);
}


// —————————————————————————————————————————


void loop() {

  // set recordingDigitIndex to 0 and clear recordingArray and recordIndex
  recordingDigitIndex = 0;
  clearRecordingArray();
  recordIndex = 1;
  
  // set record buttons to 'pressed' state
  recordButtonsToPressed();

  // wait until all record buttons are unpressed
  boolean unpressed;
  do {
    unpressed = true;
    for(int i=0; i<recordingButtons; i++) {
      buttonStates[i] = digitalRead(buttonPins[i]);
      if(buttonStates[i] == LOW) {
        unpressed = false;
      }
    }
  } while(unpressed == false);

  //Serial.println();
  //Serial.println("Void loop");
  //printButtonStates();
  
  /*
  // to be used in debouncing:
  int reading[buttons]; 
  
  unsigned long lastDebounceTime[buttons]; // the last time input was recorded from button
  for(int i=0; i<buttons; i++) {
    lastDebounceTime[i] = 0;
  }
  */

  // get state of capture button
  if (analogRead(buttonPins[captureButtonIndex]) > 0) {
    buttonStates[captureButtonIndex] = 1; 
  }
  else {
    buttonStates[captureButtonIndex] = 0; 
  }
  
  // Record button presses in array until either the 
  // array is full or the recored data is locked by
  // a press of the 'capture' button
  while (recordingDigitIndex < 5 && buttonStates[captureButtonIndex] == 0) {

  //while (recordingDigitIndex < 5) {
    
    // set last button states to button states
    for (int i=0; i < recordingButtons; i++) {
      buttonLastStates[i] = buttonStates[i];
    }
    
    // capture current button states (digital & analog separately):
    for (int i=0; i < recordingButtons; i++) {
      buttonStates[i] = digitalRead(buttonPins[i]);
    }
    
    if (analogRead(buttonPins[captureButtonIndex]) > 0) {
      buttonStates[captureButtonIndex] = 1; 
      //Serial.println("HIT");
    }
    
    // check recording buttons for new press and record if so
    for(int i=0; i < recordingButtons; i++) {
      if(buttonStates[i] != buttonLastStates[i]) {   // if button state has changed
        if(buttonStates[i] == LOW) {    
          recordingDigit = i+1;
          recordingDigitIndex++;
          //Serial.println();
          //Serial.print("Button press: ");
          //Serial.println(i+1);
          //Serial.print("recording digit: ");
          //Serial.println(recordingDigit);
          //Serial.print("recording digit index: ");
          //Serial.println(recordingDigitIndex);
        }
      }
    }
   
    // update recording array  
    recordingArray[recordingDigitIndex-1] = recordingDigit;

    delay(debounceDelay);
 
  } // end while loop

  //printRecordingArray();
  // calculate index number from recordingArray
  for(int i=0; i < recordingMaxDigits; i++) {
    recordIndex = recordIndex + recordingArray[i] * pow(10, recordingMaxDigits-i-1);
  }
  //Serial.print("Record Index: ");
  
  // Convert record index to string and send to Raspberry Pi
  String arduinoOutput = String(recordIndex);
  Serial.println(arduinoOutput);

  delay(debounceDelay);

}


// —————————————————————————————————————————


// manually set record buttons to a pressed state
void recordButtonsToPressed() {

  //digital
  for(int i=0; i<recordingButtons; i++) {
    buttonStates[i] = LOW;
  }
}

// clear recording array
void clearRecordingArray() {
  for(int i=0; i<recordingMaxDigits; i++) {
    recordingArray[i]=0;
  }
}

// print recordingArray
void printRecordingArray() {
  Serial.print("Recording array: ");
  for(int i=0; i<recordingMaxDigits; i++) {
    Serial.print(recordingArray[i]);
    Serial.print(", ");
  }
  Serial.println();
}

// print button states
void printButtonStates() {
  Serial.print("Button states: ");
  for(int i=0; i<recordingButtons; i++) {
    Serial.print(buttonStates[i]);
    Serial.print(", ");
  } 
  Serial.println();
}



/*
 * To do: 
 *  Quick-access buttons
 *  
 * 
 * 
 */


