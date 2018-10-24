#include "AnalogButton.h"


AnalogButton::AnalogButton(byte buttonPin, int buttonThreshold, unsigned long debounceTime = 50) {
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















 

