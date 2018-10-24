#ifndef AnalogButton_H
#define AnalogButton_H

#include "Arduino.h"

#define CURRENT 0
#define PREVIOUS 1
#define CHANGED 2

class AnalogButton {
  public:
    AnalogButton(byte buttonPin, int buttonThreshold, unsigned long debounceTime = 50) ;
    void adjustThreshold(int newThreshold) ;
    void adjustDebounce(unsigned long newDebounce = 50) ;
    bool isPressed() ;
    bool wasPressed() ;
    bool stateChanged() ;
    bool newPress() ;
  private:
    byte pin ;
    int threshold ;
    unsigned long lastDebounceTime ;
    byte reading ;
    byte state ;
    unsigned long debounce ;
} ;

#endif

