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
const byte addresses[][6] = {"00001", "00002"};
bool transmitting = false;

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
  modeZoom,
  modePaletteSimple,
  modeWave
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]));
uint8_t renderMode = 0;



// SETUP
// ————————————————————————————————————————————————
void setup() {
  
  delay(1000);
  
  radio.begin();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);// pipe 0 is used for writing, so we use pipe 1 so startListening() doesn't overwrite the writing pipe
  radio.setPALevel(RF24_PA_MIN);
  if (transmitting == true) {radio.stopListening();} else {radio.startListening();};
  
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
  
  // send/receive transmission
  if (transmitting == true) {sendPayload();} else {getPayload();};

  // animate
  if ( brightness > 5 ) {
    (*renderers[renderMode])();
  } else {
    clear();
  }
  show();
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

void clear() {
  FastLED.clear();
  for ( uint8_t i=0; i<NUM_LEDS_JEWEL; i++ ) {
    jewel.setPixelColor(i, jewel.Color(0, 0, 0, 0));
  }
  jewel.show();
}

void sendPayload() {
  Payload _p = { brightness, renderMode, trigger, param1, param2, param3 };
  radio.write(&_p, sizeof(_p));    
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


// ANIMATION MODES
// ————————————————————————————————————————————————

void modeZoom() {
  
  static bool active = false;
  static bool zoomForward = false;
  static uint8_t zoomHue = 0;
  static int8_t zoomIndex = NUM_LEDS_STRIP - 1;
  static uint8_t zooms = 0;
  static uint8_t zoomCount = 0;


  // new trigger
  if ( active == false && trigger == true ) {
    trigger = false;
    active = true;
    zooms = random(1,3);
    transmitting = true;
    radio.stopListening();
  }

  EVERY_N_MILLISECONDS(10) {

    fadeToBlackBy(ledsStrip, NUM_LEDS_STRIP, 20);
    if (active == true) {
      ledsStrip[NUM_LEDS_STRIP-zoomIndex-1] = CHSV(zoomHue++, 255, 255);
      if ( zoomForward) {zoomIndex++;} else {zoomIndex--;};
  
      if ( zoomIndex > NUM_LEDS_STRIP - 1 ) {
        zoomIndex--;
        zoomForward = false;
        zoomCount++;
        Serial.println(zoomCount);
        if ( zoomCount == zooms ) {
          trigger = true;
          sendPayload();
          trigger = false;
          active = false;
          zoomCount = 0;
          transmitting = false;
          radio.startListening();
        }
      }
  
      if ( zoomIndex < 0 ) {
        zoomIndex++;
        zoomForward = true;
      }
    }
  }
}


void modePaletteSimple() {
  updatePalette();
  fillFromPaletteSimple(ledsStrip, NUM_LEDS_STRIP, currentPalette);

  // jewel animation
  jewel.setBrightness( beatsin8(20, 10, brightness) );
  for ( uint8_t i=0; i<NUM_LEDS_JEWEL; i++ ) {
    jewel.setPixelColor(i, jewel.Color(81, 0, 255, 0));
  }
  
}

void modeWave() {

  // jewel animation
  jewel.setBrightness( beatsin8(20, 10, brightness*0.75) );
  for ( uint8_t i=0; i<NUM_LEDS_JEWEL; i++ ) {
    jewel.setPixelColor(i, jewel.Color(81, 0, 255, 0));
  }
  
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

  // noise params
  const uint8_t maxNoisePct = 2; // maximum noise
  const uint8_t noiseSpeedParam = 150; // lower value = faster (more variation at given wave point over time)
  static uint8_t noiseScale = 10; // higher number = choppier (more variation between parts of wave at given moment)
  static uint16_t noiseDist; // random number for noise generator
  
  const uint8_t midLed = NUM_LEDS_STRIP / 2;
  static CHSV nextCHSV;
  static unsigned long lastWaveStart = 0;

  // start new wave
  if ( trigger == true ) {
    lastWaveStart = millis();
    waveWidthParam = random8(waveWidthParamMin, waveWidthParamMax); // vary wave width
//    noiseDist = random(12345);
    trigger = false;
  }

  // animate
  for ( int i=0; i<midLed+1; i++ ) {
    unsigned long waveStart = lastWaveStart + offsetMs*i;
    uint8_t hue = beatsin8(5, hue1, hue2, 0, inoise8(i*noiseScale)); // vary the hue, with period offset for each pixel set by noise function
    while (true) {
      if ( millis() > waveStart) { // has wave started yet?
        unsigned long waveOffset = ( millis() - waveStart ) / waveWidthParam; // higher value for wider wave
        
        if ( waveOffset < 256 ) { // has wave not ended yet?
          static uint8_t power = 2;  // higher pow => longer tails:
          uint16_t param = pow(quadwave8(waveOffset), power);

          // brightness
          uint8_t b = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterBright, waveBright); // blend brightness between standing water and wave peak
          // position within wave is defined by brightness, so use that as x coord in noise function
//          int8_t noise = scale8( maxNoisePct*b*2/100, inoise8(b*noiseScale, b*noiseScale + noiseDist + millis()/noiseSpeedParam) ) - maxNoisePct*b/100;
//          b = ( noise > 0 ) ? qadd8(b, noise) : max(b + noise, waterBright); // add or subtract noise, in [waterBright, 255]

          // saturation
          uint8_t s = map(pow(quadwave8(waveOffset), power), 0, pow(255, power), waterSat, waveSat); // blend saturation between standing water and wave peak
//          s = ( noise > 0 ) ? qadd8(s, noise) : max(s + noise, waterSat); // add or subtract noise, in [waterSat, 255]
          nextCHSV = CHSV(hue, s, b);
          break;
        }
        
      }
      nextCHSV = CHSV(hue, waterSat, waterBright);
      break;
    }
    // set LEDs in both directions
    if ( midLed - i >= 0 ) {
      ledsStrip[midLed-i] = nextCHSV;
    }
    if ( midLed + i < NUM_LEDS_STRIP ) {
      ledsStrip[midLed+i] = nextCHSV;
    }        
  }    
}


// ANIMATION HELPERS
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
