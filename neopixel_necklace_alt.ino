// VERSION THAT GENERATES ARRAYS WITH THE ARRAY
// INDICES OF LIGHTS FOR BOTH STANDARD AND PULSING LIGHTS

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUM_LEDS 24
#define PULSE_LIGHTS 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KH800);

int pulse_lights[PULSE_LIGHTS];
int reg_lights[NUM_LEDS - PULSE_LIGHTS];

// PULSE CONTROL
// inputs
unsigned long pulse_delay = 40; // in ms
uint32_t pulse_color = strip.Color(0, 0, 255);
int pulse_interval = 500; // time between pulses, in ms

int pulse_light_interval = round(NUM_LEDS / PULSE_LIGHTS); // steps from one pulse light to next
int pulse_light_shift = 0;
bool pulse_prev = 0;
bool pulse_curr = 0;
bool increment_pulse_light_shift = 0;
unsigned long last_pulse_start = 0;


void setup() {

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(100);
  setLightTypes();
//  Serial.begin(9600);
}


void loop() {

  
  // pulse control
  pulse_prev = pulse_curr;
  if (pulse_curr == 1) {
    if ( millis() - last_pulse_start > pulse_delay ) {
      pulse_curr = 0;
      increment_pulse_light_shift = 1;
    }
  } else {
    if ( millis() - last_pulse_start >= pulse_interval ) {
      pulse_curr = 1;
      last_pulse_start = millis();
      Serial.println(pulse_light_shift);
    }
  }

  for(uint16_t i=0; i<PULSE_LIGHTS; i++) {
    if ( pulse_lights[i] >= 0 ) {
      if ( pulse_curr == 1 ) {
        strip.setPixelColor(pulse_lights[i], pulse_color);
      } else {
        strip.setPixelColor(pulse_lights[i], 0);
      }
    }
  }
  strip.show();
  
  if ( increment_pulse_light_shift == 1 ) {
    incrementPulseLightShift(2);
    increment_pulse_light_shift = 0;
    setLightTypes();
  }
}  


void setLightTypes() {

  int i_pulse = 0;
  int i_reg = 0;
  for ( uint16_t i=0; i<NUM_LEDS; i++ ) {
    if ( (i+pulse_light_interval-pulse_light_shift) % pulse_light_interval == 0 ) {
      pulse_lights[i_pulse] = i;
      i_pulse++;
    } else {
      reg_lights[i_reg] = i;
    }
  }

  // fill any add'l slots in arrays with -1
  for ( uint16_t i=i_pulse; i<PULSE_LIGHTS; i++ ) {
    pulse_lights[i] = -1;
  }
  for ( uint16_t i=i_reg; i<NUM_LEDS-PULSE_LIGHTS; i++ ) {
    reg_lights[i] = -1;
  }  
  
}


// Change the first light that pulses
void incrementPulseLightShift(int num) {
  pulse_light_shift += num;
  if ( pulse_light_shift >= pulse_light_interval ) {
    pulse_light_shift -= pulse_light_interval;
  }
}