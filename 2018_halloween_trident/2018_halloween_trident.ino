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
  modeWave
//  modeTest,
//  modeBrightness,
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
  
  static uint8_t j = 0;

  j = ( millis()/10 ) % 255;
//  ledsTube[8] = CHSV(215, 255, sin8(j));
  ledsTube[8] = CHSV(215, 255, cubicwave8(j));
  ledsTube[10] = CHSV(215, 255, quadwave8(j));
}


void modeWave() {
  
  const uint8_t waterHue = 190;
  const uint8_t waterSat = 255;
  const uint8_t waterBright = 30;
  
  const uint8_t waveHue = 190;
  const uint8_t waveSat = 200;
  const uint8_t waveBright = 255;
  
  const uint16_t offsetMs = 40;
  const uint16_t waveDelayMinMs = 2000;
  const uint16_t waveDelayMaxMs = 5000;

  const uint16_t maxNoise = 5;
  
  static unsigned long lastWaveStart = 0;
  static uint16_t nextWaveDelay = waveDelayMinMs;
  
  // start new wave
  if ( millis() - lastWaveStart > nextWaveDelay ) {
    lastWaveStart = millis();
    nextWaveDelay = random16(waveDelayMinMs, waveDelayMaxMs);    
  }

  // animate
  for ( unsigned long i=0; i<NUM_LEDS_TUBE; i++ ) {
    unsigned long waveStart = lastWaveStart + offsetMs*i;
    while (true) {
      if ( millis() > waveStart) { // has wave started yet?
        unsigned long waveOffset = ( millis() - waveStart ) / 6;
        
        if ( waveOffset < 256 ) { // has wave not ended yet?
          static uint8_t power = 3;  // higher pow => longer tails:
          uint16_t param = pow(quadwave8(waveOffset), power);
          uint16_t noise = map(
            inoise8(map(i, 0, NUM_LEDS_TUBE, 0, 255), millis() / 20) // space out x coords
            , 0, 255, 0, maxNoise*2    // increase or decrease by at most maxNoise
          );
          uint16_t s = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterSat, waveSat); // blend saturation between standing water and wave peak
          s += noise - maxNoise;
          if ( s > 255 ) s = 255;
          uint16_t b = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterBright, waveBright); // blend brightness between standing water and wave peak
          b += noise - maxNoise;
          if ( b > 255 ) b = 255;
          ledsTube[i] = CHSV(waterHue, s, b);
          break;
        }
        
      }
      ledsTube[i] = CHSV(waterHue, waterSat, waterBright);
      break;
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
