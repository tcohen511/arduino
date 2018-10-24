#include <Button.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

// SENSORS
#define POT_PIN 10
#define BUTTON_PIN 12
const int DEBOUNCE = 50;
Button button(BUTTON_PIN, DEBOUNCE);

Adafruit_MMA8451 accel = Adafruit_MMA8451();
sensors_event_t event;
uint8_t orientation;

// RADIO
#define CE_PIN 7
#define CSN_PIN 8
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// define struct to send pot reading and whether new button click
typedef struct {
  int pot_value;
  bool new_press;
} Payload;


void setup() {
  button.begin();

//  radio.begin();
//  radio.openWritingPipe(address);
//  radio.setPALevel(RF24_PA_MIN);
//  radio.stopListening();

  accel.begin();
  accel.setRange(MMA8451_RANGE_2_G);

  Serial.begin(115200);
}

void loop() {
  
  // read potentiometer value
  int pot_value = analogRead(POT_PIN);
//  Serial.println(pot_value);

  // check for button press
  bool new_press = button.pressed();
  if (new_press) {
    Serial.println("press");
  }

  // read accelerometer
  accel.getEvent(&event);
  orientation = accel.getOrientation();
  String o;
  switch (orientation) {
    case MMA8451_PL_PUF: 
      o = "Portrait Up Front";
      break;
    case MMA8451_PL_PUB: 
      o = "Portrait Up Back";
      break;    
    case MMA8451_PL_PDF: 
      o = "Portrait Down Front";
      break;
    case MMA8451_PL_PDB: 
      o = "Portrait Down Back";
      break;
    case MMA8451_PL_LRF: 
      o = "Landscape Right Front";
      break;
    case MMA8451_PL_LRB: 
      o = "Landscape Right Back";
      break;
    case MMA8451_PL_LLF: 
      o = "Landscape Left Front";
      break;
    case MMA8451_PL_LLB: 
      o = "Landscape Left Back";
      break;
    }
//  Serial.println(o);

  // send payload
//  Payload _p = { pot_value, new_press };
//  radio.write(&_p, sizeof(_p));  
}
