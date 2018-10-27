#include <Button.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#include "FastLED.h"
#include <Adafruit_NeoPixel.h>
#include <PoseidonGradientPalettes.h>


// RADIO
// ————————————————————————————————————————————————
#define CE_PIN 3
#define CSN_PIN 4
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

typedef struct {
  uint8_t brightness;
  bool mode;
  bool trigger;
  uint8_t param1;
  uint8_t param2;
  uint8_t param3;
} Payload;


// LEDs
// ————————————————————————————————————————————————
#define PIN_STRIP 13
#define PIN_JEWEL 7

#define NUM_LEDS_STRIP 20
#define NUM_LEDS_JEWEL 7

CRGB ledsStrip[NUM_LEDS_STRIP];
Adafruit_NeoPixel jewel;

uint8_t brightness = 100;
bool trigger = false; // for coordinated animations between components
uint8_t param1; // params for animations
uint8_t param2;
uint8_t param3;

extern const TProgmemRGBGradientPalettePtr gradientPalettes[];
extern const uint8_t gradientPaletteCount;

CRGBPalette16 currentPalette( gradientPalettes[0] );
CRGBPalette16 targetPalette( gradientPalettes[1] );
uint8_t currentPaletteNo = 1;


// RENDER MODES
// ————————————————————————————————————————————————
void (*renderers[])(void) {
  modePaletteSimple,
  modeWave
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]));
uint8_t renderMode = 1;



// SETUP
// ————————————————————————————————————————————————
void setup() {
  
  delay(1000);
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();  
  
  FastLED.addLeds<NEOPIXEL, PIN_STRIP>(ledsStrip, NUM_LEDS_STRIP);
  jewel = Adafruit_NeoPixel(NUM_LEDS_JEWEL, PIN_JEWEL, NEO_GRBW + NEO_KHZ800);
  setBrightness(brightness);
  jewel.begin();
  show();

  Serial.begin(115200);
}


// LOOP
// ————————————————————————————————————————————————
void loop() {
  
  // receive transmission
  getPayload();

  // animate
  if ( brightness > 5 ) {
    (*renderers[renderMode])();
  } else {
    FastLED.clear();
  }
  FastLED.show();
}

// METHODS
// ————————————————————————————————————————————————
void setBrightness(int brightness) {
  FastLED.setBrightness(brightness);
  jewel.setBrightness(brightness);
}

void show() {
  FastLED.show();
  jewel.show();
}

void getPayload() {

  if ( !radio.available() ) {
    return;
  }
  
  Payload _p = {};
  radio.read(&_p, sizeof(_p));

  // set LED brightness
  if ( _p.brightness != brightness ) {
    brightness = _p.brightness;
    setBrightness(brightness);
  }

  // set animation mode
  renderMode = _p.mode;
  
  // new trigger?
  trigger = _p.trigger;

  // params
  param1 = _p.param1;
  param2 = _p.param2;
  param3 = _p.param3;
}

void updatePalette() {
  EVERY_N_SECONDS( 5 ) {
    currentPaletteNo = addmod8( currentPaletteNo, 1, gradientPaletteCount);
    targetPalette = gradientPalettes[ currentPaletteNo ];
  }
//  nblendPaletteTowardPalette( currentPalette, targetPalette, 16);
  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette( currentPalette, targetPalette, 16);
  }  
}


// ANIMATION MODES
// ————————————————————————————————————————————————

void modePaletteSimple() {
  updatePalette();
  fillFromPaletteSimple(ledsStrip, NUM_LEDS_STRIP, currentPalette);
}

void modeWave() {

//  for ( uint8_t i=0; i< NUM_LEDS_STRIP; i++ ) {
//    ledsStrip[i] = CHSV(190, 255, 255);
//  }
//  return;
  
  const uint8_t hue = 190;
  const uint8_t waterSpreadPct = 5; // amount of water that spreads from one LED to next each frame
  const uint8_t waterSpreadMin = 100;
  const uint8_t framesPerSecond = 60;
  
  const uint8_t midLed = NUM_LEDS_STRIP / 2;
  static uint8_t waterVolume[midLed + 1]; // array to hold water volume amounts by LED; 0 idx is middle
  static uint8_t waterVolumeSize = sizeof(waterVolume) / sizeof(waterVolume[0]);
  
  // add new water (brightness) to middle LED
  if ( param1 > 0 ) {
    waterVolume[0] = qadd8(waterVolume[0], param1);
    ledsStrip[midLed] = CHSV(hue, 255, waterVolume[0]);
    param1 = 0;
  }

  EVERY_N_MILLISECONDS(1000 / framesPerSecond) {
  
    // decrease saturation
  
    // spread water
    for ( int i = waterVolumeSize-1; i>=0; i-- ) {
      // pass water along
      uint8_t lost = waterSpreadPct * waterVolume[i] / 100;
      if ( i == midLed) Serial.println(lost);
      waterVolume[i] = qsub8(waterVolume[i], lost);
      if ( i<waterVolumeSize-1 ) {
        waterVolume[i+1] = qadd8(waterVolume[i+1], lost);
      }
    }
    
   
    // set LEDs
    for ( uint8_t j=midLed+1; j<NUM_LEDS_STRIP-1; j++ ) {
      ledsStrip[j] = CHSV(hue, 255, waterVolume[j - midLed]);
    }
    for ( int k=midLed-1; k >= 0; k-- ) {
      ledsStrip[k] = CHSV(hue, 255, waterVolume[midLed - k]);
    }
    
//    Serial.println(waterVolume[midLed]);
  }

//  19: 9
//  18: 8
//  17: 7
//  16: 6
//  15: 5
//  14: 4
//  13: 3
//  12: 2
//  11: 1
//  10: 0
//  9: 1
//  8: 2
//  7: 3
//  6: 4
//  5: 5
//  4: 6
//  3: 7
//  2: 8
//  1: 9
//  0: 10
}


// ANIMATIONS
// ————————————————————————————————————————————————

void fillFromPaletteSimple(CRGB* ledArray, uint16_t numLeds, CRGBPalette16& palette) {
  uint8_t startIndex = millis() / 8;
  fill_palette( ledArray, numLeds, startIndex, (256 / numLeds) + 1, palette, 255, LINEARBLEND);
}


// GRADIENTS
// ————————————————————————————————————————————————
const TProgmemRGBGradientPalettePtr gradientPalettes[] = {
//  green_to_blue,
  bhw1_14_gp,
  bhw1_greeny_gp
};
const uint8_t gradientPaletteCount = 
  sizeof( gradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
