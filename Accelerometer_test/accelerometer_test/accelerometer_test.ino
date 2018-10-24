/**************************************************************************/
/*!
    @file     Adafruit_MMA8451.h
    @author   K. Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    This is an example for the Adafruit MMA8451 Accel breakout board
    ----> https://www.adafruit.com/products/2019

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

Adafruit_MMA8451 mma = Adafruit_MMA8451();

int x_min = 0;
int y_min = 0;
int z_min = 0;

int x_max = 0;
int y_max = 0;
int z_max = 0;

int delta_x = 0;
int delta_y = 0;
int delta_z = 0;

unsigned long stationary_start_time = 0;

float acc_x = 0;
float acc_y = 0;
float acc_z = 0;

float max_acc_x = 0;
float max_acc_y = 0;
float max_acc_z = 0;

float min_acc_x = 0;
float min_acc_y = 0;
float min_acc_z = 0;

void setup(void) {
  Serial.begin(9600);
  
  Serial.println("Adafruit MMA8451 test!");
  

  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_8_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
}

void loop() {
  // Read the 'raw' data in 14-bit counts
  mma.read();
  // Serial.print("X:\t"); Serial.print(mma.x); 
  // Serial.print("\tY:\t"); Serial.print(mma.y); 
  // Serial.print("\tZ:\t"); Serial.print(mma.z); 
  // Serial.println();

  // update prev positions
	// int x_prev = x;
	// int y_prev = y;
	// int z_prev = z;
  
 //  // update current positions
 //  int x = mma.x;
 //  int y = mma.y;
 //  int z = mma.z;

 //  // update maxes?
 //  if ( x_max == 0 || x > x_max ) 
 //  	x_max = x;
 //  }
 //  if ( y_max == 0 || y > y_max ) {
 //  	y_max = y;
 //  }
 //  if ( z_max == 0 || z > z_max ) {
 //  	z_max = z;
 //  }

 //  // update mins?
 //  if ( x_min == 0 || x > x_min ) {
 //  	x_min = x;
 //  }
 //  if ( y_min == 0 || y > y_min ) {
 //  	y_min = y;
 //  }
 //  if ( z_min == 0 || z > z_min ) {
 //  	z_min = z;
 //  }

 //  // update position changes
 //  delta_x = x_max - x_min;
 //  delta_y = y_max - y_min;
 //  delta_z = z_max - z_min;


  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  // update acceleration readings
  float acc_x = event.acceleration.x;
  float acc_y = event.acceleration.y;
  acc_z = event.acceleration.z;

  // update acceleration maxes and mins 
  if (acc_x > max_acc_x) {
  	max_acc_x = acc_x;
  }
  if (acc_y > max_acc_y) {
  	max_acc_y = acc_y;
  }
  if (acc_z > max_acc_z) {
  	max_acc_z = acc_z;
  }  

  if (acc_x < min_acc_x) {
  	min_acc_x = acc_x;
  }
  if (acc_y < min_acc_y) {
  	min_acc_y = acc_y;
  }
  if (acc_z < min_acc_z) {
  	min_acc_z = acc_z;
  }


  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: \t"); Serial.print(event.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(event.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(event.acceleration.z); Serial.print("\t");
  Serial.println("m/s^2 ");

  // display current maxes and mins
	Serial.print("X_max: \t"); Serial.print(max_acc_x); Serial.print("\t");
  Serial.print("Y_max: \t"); Serial.print(max_acc_y); Serial.print("\t");
  Serial.print("Z_max: \t"); Serial.print(max_acc_z); Serial.print("\t");  
  Serial.println("m/s^2 ");

	Serial.print("X_min: \t"); Serial.print(min_acc_x); Serial.print("\t");
  Serial.print("Y_min: \t"); Serial.print(min_acc_y); Serial.print("\t");
  Serial.print("Z_min: \t"); Serial.print(min_acc_z); Serial.print("\t");    
  Serial.println("m/s^2 ");
  
  /* Get the orientation of the sensor */
  // uint8_t o = mma.getOrientation();
  
  // switch (o) {
  //   case MMA8451_PL_PUF: 
  //     Serial.println("Portrait Up Front");
  //     break;
  //   case MMA8451_PL_PUB: 
  //     Serial.println("Portrait Up Back");
  //     break;    
  //   case MMA8451_PL_PDF: 
  //     Serial.println("Portrait Down Front");
  //     break;
  //   case MMA8451_PL_PDB: 
  //     Serial.println("Portrait Down Back");
  //     break;
  //   case MMA8451_PL_LRF: 
  //     Serial.println("Landscape Right Front");
  //     break;
  //   case MMA8451_PL_LRB: 
  //     Serial.println("Landscape Right Back");
  //     break;
  //   case MMA8451_PL_LLF: 
  //     Serial.println("Landscape Left Front");
  //     break;
  //   case MMA8451_PL_LLB: 
  //     Serial.println("Landscape Left Back");
  //     break;
  //   }
  Serial.println();
  delay(500);
  
}
