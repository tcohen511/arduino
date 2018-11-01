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

// define struct for sending payload
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
#define PIN_STRIP_TRIDENT 9
#define PIN_STRIP_TUBE 6

#define NUM_LEDS_TRIDENT 6
#define NUM_LEDS_TUBE 30

CRGB ledsTrident[NUM_LEDS_TRIDENT];
CRGB ledsTube[NUM_LEDS_TUBE];

#define BRIGHTNESS_STEP 2
uint8_t brightness = 0;
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
//  modeTest,
//  modeBrightness,
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]))
uint8_t renderMode = 1;



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
  if ( button.pressed() ) {
    renderMode++;
    renderMode = mod8(renderMode, N_MODES);
  }

  sendPayload();

  // animate
  if ( brightness > BRIGHTNESS_STEP + 1 ) {
    (*renderers[renderMode])();
  } else {
    FastLED.clear();
  }
  FastLED.show();
}


// METHODS
// ————————————————————————————————————————————————
void updatePalette() {
  EVERY_N_SECONDS( 10 ) {
    currentPaletteNo = addmod8( currentPaletteNo, 1, gradientPaletteCount);
    targetPalette = gradientPalettes[ currentPaletteNo ];
  }
//  nblendPaletteTowardPalette( currentPalette, targetPalette, 16);
  EVERY_N_MILLISECONDS(100) {
    nblendPaletteTowardPalette( currentPalette, targetPalette, 16);
  }  
}

void sendPayload() {
  Payload _p = { brightness, renderMode, trigger, param1, param2, param3 };
  radio.write(&_p, sizeof(_p));    
}


// ANIMATION MODES
// ————————————————————————————————————————————————
void modePaletteSimple() {
  updatePalette();
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
  
  // colors
  const uint8_t hue1 = 150;
  const uint8_t hue2 = 190;
  const uint8_t waterSat = 255;
  const uint8_t waterBright = 40;
  const uint8_t waveSat = 150;
  const uint8_t waveBright = 255;
  
  // wave params
  const uint8_t offsetMs = 75; // higher value for slower wave
  const uint8_t waveWidthParamMin = 8; // higher value for wider waves
  const uint8_t waveWidthParamMax = 13;
  static uint8_t waveWidthParam;
  const uint16_t waveDelayMinMs = 5000;
  const uint16_t waveDelayMaxMs = 7000;

  // noise params
  const uint8_t maxNoisePct = 2; // maximum noise
  const uint8_t noiseSpeedParam = 150; // lower value = faster (more variation at given wave point over time)
  static uint8_t noiseScale = 10; // higher number = choppier (more variation between parts of wave at given moment)
  static uint16_t noiseDist; // random number for noise generator
  
  static unsigned long lastWaveStart = 0;
  static uint16_t nextWaveDelay = waveDelayMinMs;
  static uint8_t lastLedCurrentBrightness = 0; // for sending water on to crown
  static uint8_t lastLedLastBrightness = 0; // for sending water on to crown
  static bool triggered = false;
  
  // start new wave
  if ( millis() - lastWaveStart > nextWaveDelay ) {
    lastWaveStart = millis();
    triggered = false;
    nextWaveDelay = random16(waveDelayMinMs, waveDelayMaxMs);
    waveWidthParam = random8(waveWidthParamMin, waveWidthParamMax); // vary wave width
    noiseDist = random(12345);
  }

  // animate
  lastLedLastBrightness = lastLedCurrentBrightness;
  param1 = 0;
  for ( int i=0; i<NUM_LEDS_TUBE; i++ ) {
    unsigned long waveStart = lastWaveStart + offsetMs*i;
    uint8_t hue = beatsin8(5, hue1, hue2, 0, inoise8(i*noiseScale)); // vary the hue, with period offset for each pixel set by noise function
    while (true) {
      if ( millis() > waveStart) { // has wave started yet?
        unsigned long waveOffset = ( millis() - waveStart ) / waveWidthParam; // higher value for wider wave
        
        if ( waveOffset < 256 ) { // has wave not ended yet?
          static uint8_t power = 3;  // higher pow => longer tails:
          uint16_t param = pow(quadwave8(waveOffset), power);

          // brightness
          uint8_t b = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterBright, waveBright); // blend brightness between standing water and wave peak
          // if last led, set payload param1 to its decrease in brightness
          if ( i == NUM_LEDS_TUBE - 1 ) {
            lastLedCurrentBrightness = b;
          }       
          // position within wave is defined by brightness, so use that as x coord in noise function
          int8_t noise = scale8( maxNoisePct*b*2/100, inoise8(b*noiseScale, b*noiseScale + noiseDist + millis()/noiseSpeedParam) ) - maxNoisePct*b/100;
          b = ( noise > 0 ) ? qadd8(b, noise) : max(b + noise, waterBright); // add or subtract noise, in [waterBright, 255]

          // saturation
          uint8_t s = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterSat, waveSat); // blend saturation between standing water and wave peak
          s = ( noise > 0 ) ? qadd8(s, noise) : max(s + noise, waterSat); // add or subtract noise, in [waterSat, 255]
          ledsTube[NUM_LEDS_TUBE-i-1] = CHSV(hue, s, b);
          break;
        }
        
      }
      ledsTube[NUM_LEDS_TUBE-i-1] = CHSV(hue, waterSat, waterBright);
      break;
    }
  }
  // set payload param1 to decrease in brightness of last LED
  if ( lastLedCurrentBrightness > lastLedLastBrightness && triggered == false ) {
    trigger = true;
  } else {
    trigger = false;
  }


  // trident
  static uint8_t tridentHue1 = 200;
//  static uint8_t tridentHue2 = 230;
  static CRGBPalette16 tridentPalette(pink_purp);
//  uint8_t tridentHue = beatsin8(10, tridentHue1, tridentHue2);
  uint8_t tridentHue = tridentHue1;
  for ( uint8_t i=0; i<NUM_LEDS_TRIDENT; i++ ) {
    
//    ledsTrident[i] = CHSV(tridentHue, 200, beatsin8(10, 10, 100, 0, i*10));
//    ledsTrident[NUM_LEDS_TRIDENT-i-1] = ColorFromPalette(tridentPalette, beatsin8(10, 10, 100, 0, i*50), 255, LINEARBLEND);
  }

  for ( uint8_t j=0; j<NUM_LEDS_TRIDENT; j++ ) {
    ledsTrident[j] = CHSV(200, 200, beatsin8(15, 50, 100));
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
  bhw1_14_gp,
  bhw1_greeny_gp
//  pink_purp
};
const uint8_t gradientPaletteCount = 
  sizeof( gradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
