/*
  Melody

 Plays a melody

 circuit:
 * 8-ohm speaker on digital pin 8

 created 21 Jan 2010
 modified 30 Aug 2011
 by Tom Igoe

This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/Tone

 */
#include "pitches.h"

int buzzerPin = 9;
int tempo = 1500;

// notes in the melody:
int melody[] = {
  NOTE_G3, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_D4, 0,
  NOTE_E4, NOTE_G4, NOTE_B4, NOTE_G4, NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_A4, 0,
  NOTE_G3, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_D4, 0,
  NOTE_E4, NOTE_G4, NOTE_E4, NOTE_D4, NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_G4, 0,
  NOTE_G4, NOTE_FS4, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_B4, NOTE_A4, NOTE_FS4,
  NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_B4,
  NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_B4, NOTE_FS4, NOTE_G4, 0, //7 (56)
  NOTE_G4, NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_D5,  NOTE_C5,  NOTE_A4,  
  NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_D4, NOTE_A4, 
  NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_E4, 0, // 10 (77)
  NOTE_G3, NOTE_B3, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_D4, 0, // (86)
  NOTE_E4, NOTE_G4, NOTE_B4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_A4, 0, //(96)
  NOTE_G3, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, 0, //(104)
  NOTE_B4, NOTE_G4, 0, NOTE_B4, NOTE_G4, 0, NOTE_B4, NOTE_G4, //(112)
  NOTE_A4, NOTE_G4, 0 // (115)
    
    
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
float noteDurations[] = {
  .625, .125, .75, .125, .5, .125, .5, .25,
  .125, .625, .125, .625, .125, .5, .125, .5, .25,  
  .625, .125, .75, .125, .5, .125, .5, .25,
  .125, .375, .125, .5, .25, .5, .125, .125, .25, .25,
  .125, .125, .5, .125, .125, .25, .375, .125,
  .5, .125, .125, .25, .375, .125,
  .5, .125, .125, .5, .25, 1, .25, //7 (56)
  .25, .25, .25, .25, .25, .375, .125,
  .5, .125, .125, .25, .375, .125,
  .5, .125, .125, .5, .125, .125, 1.25, .25, // 10 (77)
  .625, .125, .5, .25, .125, .5, .125, .5, .25, // (86)
  .125, .625, .125, .5, .125, .25, .25, .25, .5, .25, // (96)
  .625, .125, .75, .125, .5, .125, .5, .25, // (104)
  .25, .5, .075, .25, .5, .075, .625, .75, // (112)
  .875, 2.5, 2 // (115)
  
  
  
};

void setup() {
}

void loop() {
    // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 116; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    float noteDuration = tempo * noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    float pauseBetweenNotes = noteDuration * 1.0;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  }
}
