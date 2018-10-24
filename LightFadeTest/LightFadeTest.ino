int ledPin = 3 ;
int minBrightness = 10 ; 
int maxBrightness = 255 ;
int fadeAmount = 5 ;
int fadeStep = 30 ; // in ms

int brightness = 0 ;
unsigned long lastStepTime = 0 ;

void setup() {
  pinMode(ledPin, OUTPUT) ;
}

void loop() {
  lightFade() ;

}


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
