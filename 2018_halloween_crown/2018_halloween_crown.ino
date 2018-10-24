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

// define struct to send pot reading and whether new button click
typedef struct {
  int brightness;
  bool buttonPress;
} Payload;


// LEDs
// ————————————————————————————————————————————————
#define PIN_STRIP 13
#define PIN_JEWEL 7

#define NUM_LEDS_STRIP 20
#define NUM_LEDS_JEWEL 7

CRGB ledsStrip[NUM_LEDS_STRIP];
Adafruit_NeoPixel jewel;

int brightness = 100;

extern const TProgmemRGBGradientPalettePtr gradientPalettes[];
extern const uint8_t gradientPaletteCount;

CRGBPalette16 currentPalette( gradientPalettes[0] );
CRGBPalette16 targetPalette( gradientPalettes[1] );
uint8_t currentPaletteNo = 1;


// RENDER MODES
// ————————————————————————————————————————————————
void (*renderers[])(void) {
  modePaletteSimple
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]));
uint8_t renderMode = 0;



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
  if (radio.available()) {
    Payload _p = {};
    radio.read(&_p, sizeof(_p));

    Serial.println(_p.brightness);

    // set LED brightness
    if ( _p.brightness != brightness ) {
      brightness = _p.brightness;
      setBrightness(brightness);
    }
  }

  // animate
  if ( brightness > 5 ) {
    updatePalette();
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
  
  fillFromPaletteSimple(ledsStrip, NUM_LEDS_STRIP, currentPalette);
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
