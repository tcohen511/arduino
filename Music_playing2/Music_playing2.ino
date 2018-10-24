

#include "pitches.h"

int buzzPin = 9;

// set tempo (beats/minute)

int tempo = 175;

// for less legato notes, set a separation between tones
// in fraction of eight note

float noteSeparation = 0;

// Write the song
// In each array element, put the note, followed by 
// the number of eighth notes it should play for. 
// Type 0 for rests.
  
float song[][2] = {
    {NOTE_G3, 5}, {NOTE_B3, 1},  {NOTE_D4, 6},  {NOTE_E4, 1},  {NOTE_G4, 4},  {NOTE_E4, 1},  {NOTE_D4, 4},  {0, 2}, 
    {NOTE_E4, 1}, {NOTE_G4, 5},  {NOTE_B4, 1},  {NOTE_G4, 5},  {NOTE_G4, 1},  {NOTE_FS4, 4}, {NOTE_G4, 1},  {NOTE_A4, 4}, {0, 2}, 
    {NOTE_G3, 5}, {NOTE_B3, 1},  {NOTE_D4, 6},  {NOTE_E4, 1},  {NOTE_G4, 4},  {NOTE_E4, 1},  {NOTE_D4, 4},  {0, 2}, 
    {NOTE_E4, 1}, {NOTE_G4, 3},  {NOTE_E4, 1},  {NOTE_D4, 4},  {NOTE_G4, 2},  {NOTE_FS4, 4}, {NOTE_G4, 1},  {NOTE_A4, 1}, {NOTE_G4, 2}, {0, 2}, 
    {NOTE_G4, 1}, {NOTE_FS4, 1}, {NOTE_E4, 4},  {NOTE_DS4, 1}, {NOTE_E4, 1},  {NOTE_B4, 2},  {NOTE_A4, 3},  {NOTE_FS4, 1}, 
    {NOTE_E4, 4}, {NOTE_DS4, 1}, {NOTE_E4, 1},  {NOTE_FS4, 2}, {NOTE_B3, 3},  {NOTE_B3, 1}, 
    {NOTE_E4, 4}, {NOTE_DS4, 1}, {NOTE_E4, 1},  {NOTE_B4, 4},  {NOTE_FS4, 2}, {NOTE_G4, 8},  {0, 2}, 
    {NOTE_G4, 2}, {NOTE_G4, 2},  {NOTE_FS4, 2}, {NOTE_G4, 2},  {NOTE_D5, 2},  {NOTE_C5, 3},  {NOTE_A4, 1}, 
    {NOTE_G4, 4}, {NOTE_FS4, 1}, {NOTE_G4, 1},  {NOTE_A4, 2},  {NOTE_D4, 3},  {NOTE_A4, 1}, 
    {NOTE_B4, 4}, {NOTE_C5, 1},  {NOTE_B4, 1},  {NOTE_A4, 4},  {NOTE_G4, 1},  {NOTE_FS4, 1}, {NOTE_E4, 10}, {0, 2}, 
    {NOTE_G3, 5}, {NOTE_B3, 1},  {NOTE_D4, 4},  {NOTE_D4, 2},  {NOTE_E4, 1},  {NOTE_G4, 4},  {NOTE_E4, 1},  {NOTE_D4, 4}, {0, 2}, 
    {NOTE_E4, 1}, {NOTE_G4, 5},  {NOTE_B4, 1},  {NOTE_G4, 4},  {NOTE_G4, 1},  {NOTE_G4, 2},  {NOTE_FS4, 2}, {NOTE_G4, 2}, {NOTE_A4, 4}, {0, 2}, 
    {NOTE_G3, 5}, {NOTE_B3, 1},  {NOTE_D4, 6},  {NOTE_E4, 1},  {NOTE_G4, 4},  {NOTE_A4, 1},  {NOTE_B4, 4},  {0, 2}, 
    {NOTE_B4, 2}, {NOTE_G4, 4},  {0, 0.5},      {NOTE_B4, 2},  {NOTE_G4, 4},  {0, 0.5},      {NOTE_B4, 4},  {NOTE_G4, 8}, 
    {NOTE_A4, 8}, {NOTE_G4, 16}, {0, 16}  
};

// set tempo in milliseconds per eight note 
// (beats/min > min/beat > sec/beat > ms/beat > ms/eighth note)
 
float msPerEighth = 1 / (float) tempo * 60 * 1000 / 2;



void setup() {
  
  Serial.begin(9600);

}



void loop() {

  int songLength = sizeof(song) / sizeof(song[0]);
  float delayTime = noteSeparation * msPerEighth;

  // begin parsing song[] array here to speed up the note-playing for loop below
  int noteFrequencies[songLength];
  for (int i=0; i < songLength; i++) {
    noteFrequencies[i] = song[i][0];
  }

  float noteDurations[songLength];
  for (int i=0; i < songLength; i++) {
    noteDurations[i] = song[i][1] * msPerEighth - delayTime;
  }
  
  // play the song

  for (int i=0; i < songLength; i++) {  
    
    // get note frequency
    int frequency = noteFrequencies[i];
    
    // get note duration
    float duration = noteDurations[i];
    
    // play note for specified duration, followed by
    // pause for specified note separation    
    tone(buzzPin, frequency, duration);
    delay(duration + delayTime);
    
    //stop the tone 
    noTone(buzzPin);

    /*
    // print results
    Serial.print("Note Frequency: ");
    Serial.println(frequency);
    Serial.print("Note Duration (eighths): ");
    Serial.println(song[i][1]);
    Serial.print("Note Duration (ms): ");
    Serial.println(duration);
    Serial.print("Delay Time: ");
    Serial.println(delayTime);
    Serial.print("Total Time: ");
    Serial.println(duration + delayTime);
    Serial.print("MS / eighth: ");
    Serial.println((duration + delayTime) / song[i][1]);
    Serial.println(" ");
    */
  }
}
