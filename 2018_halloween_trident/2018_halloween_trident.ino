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
const byte addresses[][6] = {"00001", "00002"};
bool transmitting = true;

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
  modeZoom,
  modeSweepingDot,
  autumnalSparkle,
  modeColorFades,  
  modeBrightness,
  modePaletteMoving,
  modeWave
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]))
uint8_t renderMode = 0;



// SETUP
// ————————————————————————————————————————————————
void setup() {
  delay(1000);
  
  button.begin();
  
  accel.begin();
  accel.setRange(MMA8451_RANGE_2_G);

  radio.begin();
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]); // pipe 0 is used for writing, so we use pipe 1 so startListening() doesn't overwrite the writing pipe
  radio.setPALevel(RF24_PA_MIN);
  if (transmitting == true) {radio.stopListening();} else {radio.startListening();};

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

  // send/receive transmission
  if (transmitting == true) {sendPayload();} else {getPayload();};

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
    FastLED.setBrightness(brightness);
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
  
  static bool active = true;
  static bool zoomForward = true;
  static uint8_t zoomHue = 0;
  static int8_t zoomIndex = 0;
  static uint8_t zooms = 3;
  static uint8_t zoomCount = 0;


  // new trigger
  if ( active == false && trigger == true ) {
    trigger = false;
    active = true;
    zooms = map(random8(), 0, 255, 1, 2);
    transmitting = true;
    radio.stopListening();
  }

  EVERY_N_MILLISECONDS(10) {

    fadeToBlackBy(ledsTube, NUM_LEDS_TUBE, 20);
    if (active == true) {
      ledsTube[NUM_LEDS_TUBE-zoomIndex-1] = CHSV(zoomHue++, 255, 255);
      if ( zoomForward) {zoomIndex++;} else {zoomIndex--;};
  
      if ( zoomIndex > NUM_LEDS_TUBE - 1 ) {
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


void modeSweepingDot() {
  currentPalette = autumnal;
  
  static uint8_t hue;

  hue = beatsin8(10, 0, 20);
  
  fadeToBlackBy(ledsTube, NUM_LEDS_TUBE, 10);
  uint8_t pos = beatsin8(13, 0, NUM_LEDS_TUBE-1);
  ledsTube[pos] = CHSV(hue, 255, 255);
}

void modePaletteMoving() {
  updatePalette();
  fillFromPaletteMoving(ledsTrident, NUM_LEDS_TRIDENT, currentPalette);
  fillFromPaletteMoving(ledsTube, NUM_LEDS_TUBE, currentPalette);
}

void modePalette(){
  currentPalette = autumnal;
  fillFromPalette(ledsTube, NUM_LEDS_TUBE, currentPalette);
}

void autumnalSparkle(){
  
  currentPalette = autumnal;
  const uint8_t noiseSpeedParam = 2; // lower value = faster color changes
  static uint8_t noiseScale = 100; // higher number = choppier (more variation between LEDs)
  static uint16_t noiseDist = random(12345); // random number for noise generator
  
  static uint8_t colorIndex = 0;
  static uint8_t brightness = 0;
    
  for(uint8_t i=0; i<NUM_LEDS_TUBE; i++) {
    colorIndex = inoise8(i*noiseScale, i*noiseScale + noiseDist + millis()/noiseSpeedParam);
    brightness = beatsin8(30, 150, 255, 0, i*100);
    ledsTube[i] = ColorFromPalette( currentPalette, colorIndex, brightness, LINEARBLEND);
  }
  for(uint8_t i=0; i<NUM_LEDS_TRIDENT; i++) {
    colorIndex = inoise8(i*noiseScale, i*noiseScale + noiseDist + millis()/noiseSpeedParam);
    brightness = beatsin8(30, 150, 255, 0, i*100);
    ledsTrident[i] = ColorFromPalette( currentPalette, colorIndex, brightness, LINEARBLEND);
  }  
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


void modeColorFades() {

  const uint8_t colors[] = {150, 200, 230};
  const uint8_t color_count (sizeof(colors) / sizeof(colors[0]));
  static uint8_t color_idx = 0;
  
//  static uint8_t hue = 213;
  static uint8_t min_brightness = 0;
  static uint8_t max_brightness = 255;
  static uint8_t bpm = 60;

  static bool peaked = false; // when brightness dips below peak, we're ready for color change as soon as we've reached trough
  static uint8_t current_brightness = 0;
  static uint8_t last_brightness = 0;

  last_brightness = current_brightness;
  current_brightness = beatsin8(bpm, min_brightness, max_brightness, 0, 0);
  
  if (current_brightness < last_brightness) {
    peaked = true;
  }
  if ( current_brightness > last_brightness && peaked == true ) {
    color_idx = addmod8(color_idx, 1, color_count);
    peaked = false;
  }
  
  fill_solid(ledsTube, NUM_LEDS_TUBE, CHSV( colors[color_idx], 255, current_brightness ));
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
  // set initial wave delay (time until first wave) to min delay time
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
  // loop through LEDs
  for ( int i=0; i<NUM_LEDS_TUBE; i++ ) {
    // lastWaveStart defines when wave reaches first LED; calculate when wave reaches given LED
    unsigned long waveStart = lastWaveStart + offsetMs*i;
    uint8_t hue = beatsin8(5, hue1, hue2, 0, inoise8(i*noiseScale)); // vary the hue, with period offset for each pixel set by noise function
    while (true) {
      if ( millis() > waveStart) { // has wave reached given LED yet?
        unsigned long waveOffset = ( millis() - waveStart ) / waveWidthParam; // where along wave are we? (x/256)
        
        if ( waveOffset < 256 ) { // has wave not passed given LED yet?
          static uint8_t power = 3;  // higher pow => longer tails:
          uint16_t param = pow(quadwave8(waveOffset), power);

          // brightness
          uint8_t b = map(param, 0, pow(255, power), waterBright, waveBright); // blend brightness between standing water and wave peak
          // if last led, store current brightness to determine trigger
          if ( i == NUM_LEDS_TUBE - 1 ) {
            lastLedCurrentBrightness = b;
          }       
          // add noise
          // position within wave is defined by brightness, so use that as x coord in noise function
          int8_t noise = scale8( maxNoisePct*b*2/100, inoise8(b*noiseScale, b*noiseScale + noiseDist + millis()/noiseSpeedParam) ) - maxNoisePct*b/100;
          b = ( noise > 0 ) ? qadd8(b, noise) : max(b + noise, waterBright); // add or subtract noise, in [waterBright, 255]

          // saturation
          uint8_t s = map(param, 0, pow(255, power), waterSat, waveSat); // blend saturation between standing water and wave peak
          s = ( noise > 0 ) ? qadd8(s, noise) : max(s + noise, waterSat); // add or subtract noise, in [waterSat, 255]
          ledsTube[NUM_LEDS_TUBE-i-1] = CHSV(hue, s, b);
          break;
        }
        
      }
      // if no wave at LED: set to ambient water color
      ledsTube[NUM_LEDS_TUBE-i-1] = CHSV(hue, waterSat, waterBright);
      break;
    }
  }
  // when wave has reached last LED, trigger wave start in crown
  if ( lastLedCurrentBrightness > lastLedLastBrightness && triggered == false ) {
    trigger = true;
    triggered = true;
  } else {
    trigger = false;
  }


  // trident spikes
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


// ANIMATION HELPRS
// ————————————————————————————————————————————————

void updatePalette() {
  EVERY_N_SECONDS( 10 ) {
    currentPaletteNo = addmod8( currentPaletteNo, 1, gradientPaletteCount);
    targetPalette = gradientPalettes[ currentPaletteNo ];
  }
  EVERY_N_MILLISECONDS(100) {
    nblendPaletteTowardPalette( currentPalette, targetPalette, 16);
  }  
}


void fillFromPalette(CRGB* ledArray, uint16_t numLeds, CRGBPalette16& palette) {
  fill_palette( ledArray, numLeds, 0, (256 / numLeds) + 1, palette, 255, LINEARBLEND);
}

void fillFromPaletteMoving(CRGB* ledArray, uint16_t numLeds, CRGBPalette16& palette) {
  uint8_t startIndex = millis() / 8;
  fill_palette( ledArray, numLeds, startIndex, (256 / numLeds) + 1, palette, 255, LINEARBLEND);
}





// GRADIENTS
// ————————————————————————————————————————————————

const TProgmemRGBGradientPalettePtr gradientPalettes[] = {
//  green_to_blue,
  bhw1_14_gp,
  bhw1_greeny_gp
//  pink_purp
};
const uint8_t gradientPaletteCount = 
  sizeof( gradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
