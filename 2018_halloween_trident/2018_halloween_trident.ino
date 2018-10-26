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

//  EVERY_N_MILLISECONDS(2000) {
//    Serial.println( (byte) 270 );
//  }
//  return;
  
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
  const uint8_t waterBright = 40;
  
  const uint8_t waveHue = 190;
  const uint8_t waveSat = 150;
  const uint8_t waveBright = 255;
  
  const uint16_t offsetMs = 100; // higher value for slower wave
  const uint8_t waveWidthParam = 15; // higher value for wider waves
  const uint16_t waveDelayMinMs = 6000;
  const uint16_t waveDelayMaxMs = 7000;

  const uint8_t maxNoisePct = 5; // maximum noise
  const uint8_t noiseSpeedParam = 10; // lower value = faster (more variation at given wave point over time)
  static uint16_t noiseScale = 20; // higher number = choppier (more variation between parts of wave at given moment)
  static uint16_t noiseDist; // random number for noise generator
  
  static unsigned long lastWaveStart = 0;
  static uint16_t nextWaveDelay = waveDelayMinMs;
  
  // start new wave
  if ( millis() - lastWaveStart > nextWaveDelay ) {
    lastWaveStart = millis();
    nextWaveDelay = random16(waveDelayMinMs, waveDelayMaxMs);
    noiseDist = random(12345);
  }

  // animate
  for ( unsigned long i=0; i<NUM_LEDS_TUBE; i++ ) {
    unsigned long waveStart = lastWaveStart + offsetMs*i;
    while (true) {
      if ( millis() > waveStart) { // has wave started yet?
        unsigned long waveOffset = ( millis() - waveStart ) / waveWidthParam; // higher value for wider wave
        
        if ( waveOffset < 256 ) { // has wave not ended yet?
          static uint8_t power = 3;  // higher pow => longer tails:
          uint16_t param = pow(quadwave8(waveOffset), power);
          
          uint8_t b = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterBright, waveBright); // blend brightness between standing water and wave peak
          // position within wave is defined by brightness, so use that as x coord in noise function
          int8_t noise = scale8( maxNoisePct*b*2/100, inoise8(b*noiseScale, b*noiseScale + noiseDist + millis()/noiseSpeedParam) ) - maxNoisePct*b/100; // noise in [-maxNoise, maxNoise]
          Serial.print(b);
          Serial.print("; ");
          Serial.print(noise);
          Serial.print("; ");
          b = ( noise > 0 ) ? qadd8(b, noise) : max(b + noise, waterBright); // add or subtract noise, in [waterBright, 255]
          Serial.println(b);

          // saturation
          uint8_t s = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterSat, waveSat); // blend saturation between standing water and wave peak
          s = ( noise > 0 ) ? qadd8(s, noise) : max(s + noise, waterSat); // add or subtract noise, in [waterSat, 255]
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
