/*
RemoteControl.ino
C.McCormack 4-15-17

Translates data from MATLAB GUI to 
*/

#include <Wire.h>
#define ADDRESS 8
int X = 0; Y = 0; input = 0;

void setup() {
  Wire.begin(); // join i2c bus
  Serial.begin(9600);       // opens serial port, sets data rate to 9600 baud
  Serial.setTimeout(10);    // sets time-out to 10ms after last byte received
}


void loop() {
  while (Serial.available() == 0);

  Wire.beginTransmission(ADDRESS);   // transmit to device #ADDRESS
  
  input = Serial.parseInt(); 
  switch (input) {
    case 1: // Forward
      X = 0; Y = 128;
      break;
    case 2: // Backward
      X = 0; Y = -128;
      break;
    case 3: // Left
      X = -128; Y = 0;
      break;
    case 4: // Right
      X = 128; Y = 0;
      break;
    case 5: // Forward-Left
      X = -128; Y = 128;
      break;
    case 6: // Forward-Right
      X = 128; Y = 128;
      break;
    case 7: // Back-Left
      X = -128; Y = -128;
      break;
    case 8: // Back-Right
      X = 128; Y = 128;
      break;
    case 9: // STOP
      X = 0; Y = 0;
    break;
    default: 
      X = 0; Y = 0;
    break;
  }

  int X_JOY = min(X+128,255)
  int Y_JOY = min(Y+128,255)
  
  Wire.write(X_JOY);
  Wire.write(Y_JOY);
  Wire.endTransmission();     // stop transmitting
  
}


