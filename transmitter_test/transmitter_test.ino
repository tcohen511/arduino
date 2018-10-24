/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

RF24 radio(1, 0); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);
  printf_begin(); 
  
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.printDetails();
}

void loop() {
  Serial.println(F("Now sending..."));
  
  const char text[] = "Hello World";
  if ( radio.write(&text, sizeof(text)) ) {
    Serial.println("success");
  } else {
    Serial.println("failed");
  }
  delay(1000);
}
