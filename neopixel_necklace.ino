#define NUM_LEDS 24
#define PULSE_LIGHTS 6


void setup() {
	
	pulse_light_interval = round(NUM_LEDS / PULSE_LIGHTS);
}


void loop() {

	for (int i=0; i<NUM_LEDS; i++ ) {
		
		// pulse
		if ( (i+pulse_light_interval) % pulse_light_interval == 0 ) {
			// pulse
		} 
		// animate
		else {
			// animate
		}
	}
}



// determine tempo with taps

