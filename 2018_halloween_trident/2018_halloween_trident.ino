#include <Button.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#include "FastLED.h"
#include <PoseidonGradientPalettes.h>


// SENSORS
// ————————————————————————————————————————————————
#define PIN_POT 10
#define PIN_BUTTON 12
const int DEBOUNCE = 50;

Button button(PIN_BUTTON, DEBOUNCE);
bool buttonPress = 0;

Adafruit_MMA8451 accel = Adafruit_MMA8451();
sensors_event_t event;
uint8_t orientation;


// RADIO
// ————————————————————————————————————————————————
#define CE_PIN 1
#define CSN_PIN 0
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// define struct to send pot reading and whether new button click
typedef struct {
  int brightness;
  bool buttonPress;
} Payload;


// LEDs
// ————————————————————————————————————————————————
#define PIN_STRIP_TRIDENT 9
#define PIN_STRIP_TUBE 6

#define NUM_LEDS_TRIDENT 6
#define NUM_LEDS_TUBE 30

CRGB ledsTrident[NUM_LEDS_TRIDENT];
CRGB ledsTube[NUM_LEDS_TUBE];

#define BRIGHTNESS_STEP 2
int brightness = 0;

extern const TProgmemRGBGradientPalettePtr gradientPalettes[];
extern const uint8_t gradientPaletteCount;

CRGBPalette16 currentPalette( gradientPalettes[0] );
CRGBPalette16 targetPalette( gradientPalettes[1] );
uint8_t currentPaletteNo = 1;


// RENDER MODES
// ————————————————————————————————————————————————
void (*renderers[])(void) {
//  modePaletteSimple,
//  modeWave,
//  modeTest,
//  modeBrightness,
  modeWave2
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]));
uint8_t renderMode = 0;



// SETUP
// ————————————————————————————————————————————————
void setup() {
  delay(1000);
  
  button.begin();
  
  accel.begin();
  accel.setRange(MMA8451_RANGE_2_G);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  FastLED.addLeds<NEOPIXEL, PIN_STRIP_TRIDENT>(ledsTrident, NUM_LEDS_TRIDENT);
  FastLED.addLeds<NEOPIXEL, PIN_STRIP_TUBE>(ledsTube, NUM_LEDS_TUBE);
  FastLED.setBrightness(brightness);

  FastLED.clear();
  FastLED.show();

  Serial.begin(115200);
}


// LOOP
// ————————————————————————————————————————————————
void loop() {
  
  EVERY_N_MILLISECONDS(10) {
    // read potentiometer value and update brightness
    int potValue = analogRead(PIN_POT);
    int brightnessReading = map(potValue, 0, 1023, 0, 255);
    if ( abs(brightnessReading - brightness) > BRIGHTNESS_STEP ) {
      brightness = brightnessReading;
      FastLED.setBrightness(brightness);
    }
  }

  // read accelerometer
  accel.getEvent(&event);
  orientation = accel.getOrientation();

  // check for button press
  buttonPress = button.pressed();

  sendPayload();

  // animate
  if ( brightness > BRIGHTNESS_STEP + 1 ) {
//    updatePalette();
    (*renderers[renderMode])();
  } else {
    FastLED.clear();
  }
  FastLED.show();
}


// METHODS
// ————————————————————————————————————————————————
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

void sendPayload() {
  Payload _p = { brightness, buttonPress };
  radio.write(&_p, sizeof(_p));    
}


// ANIMATION MODES
// ————————————————————————————————————————————————
void modePaletteSimple() {
  
  fillFromPaletteSimple(ledsTrident, NUM_LEDS_TRIDENT, currentPalette);
  fillFromPaletteSimple(ledsTube, NUM_LEDS_TUBE, currentPalette);
}


void modeBrightness() {
  static int hue = 213;
  static int max_brightness = 150;
  static int bpm = 30;
  
  for(uint8_t i=0; i<NUM_LEDS_TUBE; i++) {
    
    int value = beatsin8(bpm, max_brightness/2, max_brightness, 0, round( ((float)i+1) / NUM_LEDS_TUBE * 255 ));
//    int value = beatsin8(bpm, max_brightness/2, max_brightness, 0, scale8(i+1);
    ledsTube[i] = CHSV( hue, 255, value );
  }
}

void modeTest() {

  CHSV defaultColor = CHSV(45*255, 34*255, 76*255);
  
  for ( uint8_t i=0; i<NUM_LEDS_TUBE; i++ ) {
    ledsTube[i] = CRGB::Yellow;
  }  
  for ( uint8_t k=0; k<NUM_LEDS_TRIDENT; k++ ) {
    ledsTrident[k] = CRGB::Yellow;
  }  
}

void modeWave2() {

  static uint8_t offset = 0;
  offset = addmod8( offset, 10, 255);
  
  for ( uint8_t k=0; k<NUM_LEDS_TUBE; k++ ) {
    uint8_t idx = addmod8( map(k, 0, NUM_LEDS_TUBE, 0, 255), offset, 255 );
//    uint8_t idx = beatsin8(bpm, 0, 255, 0, map(k, 0, NUM_LEDS_TUBE, 0, 255));
    ledsTube[k] = ColorFromPalette( currentPalette, idx );
  }

//  uint8_t k = 12;
//  uint8_t idx = beatsin8(bpm, 0, 255, 0, map(k, 0, NUM_LEDS_TUBE, 0, 255));
//  ledsTube[k] = ColorFromPalette( currentPalette, idx );  
}

void modeWave() {
  
  const uint8_t hue = 215; // ocean blue
  const CHSV defaultColor = CHSV(122, 255, 30);
  const uint8_t waveSpread = 5; // num leds in either direction from peak of wave
  // round( (waveSpread/NUM_LEDS_TUBE)*256 - 1 )
  const uint16_t timeBetweenWavesMs = 2000;
  const uint16_t framesPerSecond = 50;
  static uint8_t maxValue = 255; // highest intensity of leds in wave (at peak)
  static uint8_t minValue = 1; // lowest intensity of leds in wave

  static uint8_t wavePeakStartIndex = waveSpread;
  static CHSV colors[NUM_LEDS_TUBE + waveSpread*2]; // array of wave intensity values at each LED; extra indices for offscreen values (parts of wave that haven't reached first LED)
  static uint8_t colorCount = sizeof(colors) / sizeof( CHSV ); // store size of value array
  static uint8_t valueDelta = round(maxValue-minValue) / waveSpread; // change in intensity from one LED to next
  
  EVERY_N_MILLISECONDS(1000 / framesPerSecond) {
    // wave moves forward
    for ( uint8_t i = colorCount - 1; i>0; i-- ) {
      colors[i] = colors[i-1];
    }
    colors[0] = defaultColor;
  
    // new wave
    EVERY_N_MILLISECONDS(timeBetweenWavesMs) {
      uint8_t v = maxValue;
      // peak
      colors[wavePeakStartIndex] = CHSV(hue, v, v);
      // spread in both directions
      for ( uint8_t j=1; j<= waveSpread; j++) {
        v -= valueDelta;
        colors[wavePeakStartIndex + j] = CHSV(hue, v, v);
        colors[wavePeakStartIndex - j] = CHSV(hue, v, v);
      }
    }
  
    // fill LEDs from wave colors
    for ( uint8_t k=0; k<NUM_LEDS_TUBE; k++ ) {
      ledsTube[k] = colors[k + waveSpread*2]; 
    }
  }
      
}


// ANIMATIONS
// ————————————————————————————————————————————————

void fillFromPaletteSimple(CRGB* ledArray, uint16_t numLeds, CRGBPalette16& palette) {
  uint8_t startIndex = millis() / 8;
  fill_palette( ledArray, numLeds, startIndex, (256 / numLeds) + 1, palette, 255, LINEARBLEND);
}


// GRADIENTS
// ————————————————————————————————————————————————

//DEFINE_GRADIENT_PALETTE( green_to_blue ) {
//    0,   0,  0,  0,
//   39,   7, 55,  8,
//   
//   99,  42,255, 45,
//  119,   2, 25,216,
//  145,   7, 10, 99,
//  186,  15,  2, 31,
//  233,   2,  1,  5,
//  255,   0,  0,  0};
//
//DEFINE_GRADIENT_PALETTE( bhw1_14_gp ) {
//    0,   0,  0,  0,
//   12,   1,  1,  3,
//   53,   8,  1, 22,
//   80,   4,  6, 89,
//  119,   2, 25,216,
//  145,   7, 10, 99,
//  186,  15,  2, 31,
//  233,   2,  1,  5,
//  255,   0,  0,  0};
//
//DEFINE_GRADIENT_PALETTE( bhw1_greeny_gp ) {
//    0,   0,  0,  0,
//   39,   7, 55,  8,
//   99,  42,255, 45,
//  153,   7, 55,  8,
//  255,   0,  0,  0};  

const TProgmemRGBGradientPalettePtr gradientPalettes[] = {
//  green_to_blue,
//  bhw1_14_gp,
//  bhw1_greeny_gp,
  wave
};
const uint8_t gradientPaletteCount = 
  sizeof( gradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
