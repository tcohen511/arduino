#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// LEDs
#define RED_PIN 6
#define GREEN_PIN 5
#define BLUE_PIN 3
#define MODE_BLUE 0
#define MODE_RED 1
int current_mode = MODE_BLUE;
int brightness;

// RADIO
#define CE_PIN 3
#define CSN_PIN 4
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// define struct to send pot reading and whether new button click
typedef struct {
  int brightness;
  bool buttonPress;
} Payload;

void setup() {
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.begin(115200);
}

void loop() {

  // receive transmission
  if (radio.available()) {
    Payload _p = {};
    radio.read(&_p, sizeof(_p));

    Serial.println(_p.brightness);

    return;

    // change mode on button press
    if ( _p.buttonPress ) {
      toggleMode();
    }
    
    // set LED brightness
    brightness = _p.brightness;


    // write to LED
    if ( current_mode == MODE_BLUE ) {
      ledBlue(); 
    } else {
      ledRed();
    }    
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
