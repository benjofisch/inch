
#include "Arduino.h"
#include <Wire.h>

#include "SoftwareSerial.h"

// Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)

SoftwareSerial port(12, 13);
#define pcSerial SERIAL_PORT_MONITOR
#define ADDRESS 13
#define DEBUG 1

#include "EasyVR.h"
EasyVR easyvr(port);
int group;
int idx;
int word1, word2, word3;
int x, y;
double scale;
int8_t lang = 0;
int command[4];
int x_joy = 128, y_joy = 128;

void setup() {
  pcSerial.begin(9600);
  port.begin(9600);
  Wire.begin(ADDRESS);
  Wire.onRequest(requestEvent);
  TWAR = (MY_ADDRESS << 1) | 1;
  // run normally
  pcSerial.println(F("---"));
  pcSerial.println(F("Bridge not started!"));

  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    delay(1000);
  }
  easyvr.setPinOutput(EasyVR::IO1, LOW);
  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.println(easyvr.getID());
  easyvr.setTimeout(1);
  lang = EasyVR::ENGLISH;
  easyvr.setLanguage(lang);

  const char* ws0[] =
  {
    "ROBOT",
  };
  const char* ws1[] =
  {
    "ACTION",
    "MOVE",
    "TURN",
    "RUN",
    "LOOK",
    "ATTACK",
    "STOP",
    "HELLO",
  };
  const char* ws2[] =
  {
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "FORWARD",
    "BACKWARD",
  };
  const char* ws3[] =
  {
    "ZERO",
    "ONE",
    "TWO",
    "THREE",
    "FOUR",
    "FIVE",
    "SIX",
    "SEVEN",
    "EIGHT",
    "NINE",
    "TEN",
  };

  const char** ws[] = { ws0, ws1, ws2, ws3 };
  easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)
  pcSerial.println("Waiting for activation");
  idx = -1;
  while (idx != 0) {
    easyvr.recognizeWord(0);
    while (!easyvr.hasFinished());
    idx = easyvr.getWord();
  }
  pcSerial.println(idx);
  idx = -1;
  while (idx != 0) {
    easyvr.recognizeWord(1);
    while (!easyvr.hasFinished());
    idx = easyvr.getWord();
  }
  pcSerial.println("VR on and ready");
  scale = .5;
}

void loop() {

  easyvr.recognizeWord(1);
  while (!easyvr.hasFinished());
  word1 = easyvr.getWord();
  if(word1 == 1){
    Serial.print("move ");
    }
  if(word1 == 2){
    Serial.print("turn ");
    }
  if(word1 == 3){
    Serial.print("run ");
    }
  if(word1 == 6){
    Serial.print("stop ");
    }
  switch (word1) {              //First word
    case 1:                         //Move
      easyvr.recognizeWord(2);
      while (!easyvr.hasFinished());
      word2 = easyvr.getWord();
//      Serial.println(word2);
      switch (word2) {
        case 4:                         //Forward
          x = 0;
          y = 128;
          Serial.println("moveForward");
          break;
        case 5:                         //Backward
          x = 0;
              y = -128;
          Serial.println("moveBackward");
          break;
      }
      break;
//      action(x,y,scale);
    case 2:                         //Turn
      easyvr.recognizeWord(2);
      while (!easyvr.hasFinished());
      word2 = easyvr.getWord();
//      Serial.println(word2);
      switch (word2) {
        case 0:                         //Left
          x = -128;
          y = 0;
          Serial.println("turnLeft");
          break;
        case 1:                         //Right
          x = 127;
          y = 0;
          Serial.println("turnRight");
          break;
      }
      break;
//      action(x,y,scale);
    case 3:
      easyvr.recognizeWord(3);
      while (!easyvr.hasFinished());
      word2 = easyvr.getWord();
      Serial.println(word2);
      switch (word2) {
        case 0:
          scale = 0;
          break;
        case 1:
          scale = .1;
          break;
        case 2:
          scale = .2;
          break;
        case 3:
          scale = 0.3;
          break;
        case 4:
          scale = 0.4;
          break;
        case 5:
          scale = 0.5;
          break;
        case 6:
          scale = 0.6;
          break;
        case 7:
          scale = 0.7;
          break;
        case 8:
          scale = 0.8;
          break;
        case 9:
          scale = 0.9;
          break;
        case 10:
          scale = 1;
          break;
      }
//      action(x,y,scale);
      break;
    case 6:
//      action(0,0,scale);
      break;
  }
}

//void action(int x, int y, double scale) {
//  int x_joy = 128 + x * scale;
//  int y_joy = 128 + y * scale;
//  Wire.write(x_joy);
//  Wire.write(y_joy);
//}
void requestEvent(){
  x_joy = 128 + x * scale;
  y_joy = 128 + y * scale;
  Wire.write(x_joy);
  Wire.write(y_joy);
  
  }
