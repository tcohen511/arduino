#include <FastLED.h>
#include <AnalogButton.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DATA_PIN 6
#define TAPPER_PIN 10
#define NUM_LEDS 24
#define PULSE_LIGHTS 6
#define STD_LIGHTS (NUM_LEDS - PULSE_LIGHTS)

#define STD_MODE 0
#define PULSE_MODE 1


// LED ARRAYS
// ————————————————————————————————————————————————
CRGB leds[NUM_LEDS];
int pulse_lights[PULSE_LIGHTS];
int std_lights[STD_LIGHTS];


// RENDER MODES
// ————————————————————————————————————————————————
void (*renderers[])(void) {
  mode_brightness, 
  mode_sparkle
};
#define N_MODES (sizeof(renderers) / sizeof(renderers[0]));
uint8_t render_mode = 0;


// SENSORS
// ————————————————————————————————————————————————

// inputs
const float analogThreshold = 0.25 ; // voltage at which to register tap
unsigned long debounceDelay = 30 ; // ms

AnalogButton tapper = AnalogButton(TAPPER_PIN, analogThreshold, debounceDelay);


// TAPPING INPUT
// ————————————————————————————————————————————————
const int TAPS_REQ = 4;
unsigned long tap_expiration = 2000; // ms before taps from unfinished pulse initiation are cleared
unsigned long doubletap_threshold = 250; // time between taps below which a double tap is registered

unsigned long tap_intervals[TAPS_REQ-1]; // store initiating taps, to be averaged; 4 taps -> 3 intervals
int tap_count = 0;
unsigned long last_tap = 0;


// PULSE CONTROL
// ————————————————————————————————————————————————

// inputs
unsigned long pulse_delay = 40; // in ms
uint32_t pulse_color = CRGB::Yellow;
int pulse_interval = 500; // time between pulses, in ms

byte current_mode = STD_MODE;
int pulse_light_interval = round(NUM_LEDS / PULSE_LIGHTS); // steps from one pulse light to next
int pulse_light_shift = 0;
bool increment_pulse_light_shift = 0;
bool pulse_prev = 0;
bool pulse_curr = 0;
unsigned long pulse_count = 0;
unsigned long last_pulse_start = 0;
unsigned long correction_tap_time = 0;


// ANIMATIONS
// ————————————————————————————————————————————————

unsigned long animation_start = 0;


// SETUP
// ————————————————————————————————————————————————
void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(200);
  setLightTypes();
  Serial.begin(19200);
}


// LOOP
// ————————————————————————————————————————————————
void loop() {
  
  // animation
  EVERY_N_MILLISECONDS(5000) { 
    render_mode++; // rotate animation mode
    render_mode %= N_MODES; // wrap around to start
  };
  (*renderers[render_mode])();
  
  // expire unfinished taps
  if ( current_mode == STD_MODE ) {
    if ( tap_count > 0 && ( millis() - last_tap > tap_expiration ) ) {
      // clear taps and recorded intervals
      tap_count = 0;
      for ( int i=0; i<TAPS_REQ-1; i++ ) {
        tap_intervals[i] = 0;
      }
    }
  }
  
  // tapper
  if ( tapper.newPress() ) {
    if ( current_mode == STD_MODE ) {
      
      tap_count++;
      // record interval
      if ( tap_count > 1 ) {
        tap_intervals[tap_count - 2] = millis() - last_tap;
      }
      last_tap = millis();

      // calc blended interval and enter pulse mode
      if ( tap_count == TAPS_REQ ) {
        unsigned long sum = 0L;
        for ( int i=0; i<TAPS_REQ-1; i++ ) {
          sum += tap_intervals[i];
          // clear array
          tap_intervals[i] = 0;
        }
        pulse_interval = round( sum / (TAPS_REQ - 1) );
        pulse_count = 0;
        current_mode = PULSE_MODE;
        setLightTypes();
      }

      // start new pulse to correspond with tap
      last_pulse_start = last_tap;
      pulse_curr = 1;      
      
    } else if ( current_mode == PULSE_MODE ) {
      
      // if double tap, return to standard mode
      if ( millis() - correction_tap_time < doubletap_threshold ) {
        current_mode = STD_MODE;
        setLightTypes();
        return;
      }
            
      // tempo correction
      correction_tap_time = millis();
      
      // determine which beat correction tap is correcting: prev or next
      bool prev = correction_tap_time - last_pulse_start < pulse_interval/2;
      if ( prev == 0 ) {
        pulse_count++;
      }

      // update interval
      pulse_interval = round((correction_tap_time - last_tap) / pulse_count);   

      // start new pulse to correspond with tap
      last_pulse_start = correction_tap_time;
      pulse_curr = 1;
    }

  }
  
  // pulse control
  pulse_prev = pulse_curr;
  if ( current_mode == PULSE_MODE || pulse_prev ) {
    if (pulse_prev == 1) {
      if ( millis() - last_pulse_start > pulse_delay ) {
        pulse_curr = 0;
        increment_pulse_light_shift = 1;
      }
    } else {
      if ( millis() - last_pulse_start >= pulse_interval ) {
        pulse_count++;
        pulse_curr = 1;
        last_pulse_start = millis();
      }
    }
  }
  
  // set pulse lights
  for(uint16_t i=0; i<NUM_LEDS; i++) {
    
    // pulse
    if ( (i+pulse_light_interval-pulse_light_shift) % pulse_light_interval == 0 ) {
      if ( pulse_curr == 1 ) {
        leds[i] = pulse_color;
      } else {
        leds[i] = 0;
      }
    } 
  }
  FastLED.show();
  
  if ( increment_pulse_light_shift == 1 ) {
//    incrementPulseLightShift(1);
    increment_pulse_light_shift = 0;
  }
}


// METHODS
// ————————————————————————————————————————————————

void mode_brightness() {
  static int hue = 213;
  static int max_brightness = 150;
  static int bpm = 45;
  
  for(uint8_t i=0; i<STD_LIGHTS; i++) {
    
    int value = beatsin8(bpm, max_brightness/2, max_brightness, 0, round( ((float)i+1) / STD_LIGHTS * 255 ));
    leds[std_lights[i]] = CHSV( hue, 255, value );
  }
}

void mode_rainbow() {

  for(uint8_t i=0; i<STD_LIGHTS; i++) {
    leds[std_lights[i]] = CHSV(round((float) i / STD_LIGHTS * 256) - 1, 255, 100);
  }
}

void mode_sparkle() {
  int p = 20; // every p milliseconds
  static uint8_t random_pixel = 0;
  static unsigned long last_sparkle = 0;
  
  if( (millis() - last_sparkle) > p ) {
    clear_std_lights();
    uint8_t r;
    do {
      r = random(STD_LIGHTS);                // Pick a new random pixel
    } while(r == random_pixel);       // but not the same as last time
    random_pixel = r;                 // Save new random pixel index
    leds[std_lights[random_pixel]] = CRGB::Blue;
    last_sparkle = millis();
  }
}


// Rotating color wheel, using 'raw' RGB values (no gamma correction).
// Average current use is about 1/2 of the max-all-white case.
void mode_colorwheel() {
  uint32_t t = millis();
  for(uint8_t i=0; i<STD_LIGHTS; i++) {
    leds[std_lights[i]] = CHSV(t + i * 1530 / 12, 255, 50);
  }  
}

void clear_std_lights() {
  for ( uint16_t i=0; i<STD_LIGHTS; i++ ) {
    leds[std_lights[i]] = 0;
  }
}

void setLightTypes() {

  int i_pulse = 0;
  int i_std = 0;
  for ( uint16_t i=0; i<NUM_LEDS; i++ ) {
    if ( (i+pulse_light_interval-pulse_light_shift) % pulse_light_interval == 0 ){
      pulse_lights[i_pulse] = i;
      i_pulse++;
    } else {
      std_lights[i_std] = i;
      i_std++;
    }
  }
}


// Change the first light that pulses
void incrementPulseLightShift(int num) {
  pulse_light_shift += num;
  if ( pulse_light_shift >= pulse_light_interval ) {
    pulse_light_shift -= pulse_light_interval;
  }
}
