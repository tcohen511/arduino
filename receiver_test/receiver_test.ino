/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

RF24 radio(3, 4); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);
  printf_begin(); 
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  radio.printDetails();
}

void loop() {
  if (radio.available()) {
    Serial.print("Got transmission: ");
    
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
}
