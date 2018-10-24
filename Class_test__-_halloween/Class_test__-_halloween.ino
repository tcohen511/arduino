#define CURRENT 0
#define PREVIOUS 1
#define CHANGED 2

// Class member variables
byte pin = A10;
int threshold =2;
unsigned long debounce = 60;

// State variables
unsigned long lastDebounceTime = 0;
byte reading = 0;
byte state = 0;

int pressNumber = 0 ;



void setup() {
  bitWrite(state,CURRENT,1) ;
  bitWrite(reading,CURRENT,1) ;

  Serial.begin(9600) ;

}

void loop() {
  if (newPress()) {
    Serial.print("press ");
    Serial.print(pressNumber);
    Serial.print(":");
    Serial.println(analogRead(pin)) ;
    pressNumber++;  
  }  

}




bool isPressed() {
  // update variables
  bitWrite(reading,PREVIOUS,bitRead(reading,CURRENT)) ;
  bitWrite(state,PREVIOUS,bitRead(state,CURRENT)) ;
  
    /* Test whether reading has changed since last time; 
     * if so, reset debounce time.
     */
    int newReading = analogRead(pin) ;
    bitWrite(reading,CURRENT, (abs(newReading) > threshold)) ;
    if (bitRead(reading,CURRENT) != bitRead(reading,PREVIOUS)) {
      lastDebounceTime = millis();
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


 bool stateChanged() {
  if (bitRead(state,CHANGED)) {
    return true;
  } else {
    return false;
  }
 }


 bool newPress() {
  return (isPressed() && stateChanged());
 }
