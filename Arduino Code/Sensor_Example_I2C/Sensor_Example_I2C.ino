// ******************
// Sensor_Example_I2C.ino
// B. Fischer
// 3-28-2017

// Sensor UNO joins i2c bus with specified address,
// receiving data from and transmitting data to to
// the primary.
// ******************
// Connections
// 
// i2c over 1/8" cable
// SDA - tip
// SCL - ring
// GND - sleeve
// ******************
// Libraries

#include <Wire.h>

// ******************
// Definitions

#define ADDRESS 8
#define DEBUG 1

// ******************
// Global Variables

int x_joy, y_joy; // joystick variables
int x, y;

void setup() {
  // begin serial transmission at 115200 baud
  Serial.begin(115200);

  // join i2c bus at specified address
  Wire.begin(ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  TWAR = (MY_ADDRESS << 1) | 1;  // enable broadcasts to be received
}

void loop() {
  // I recommend checking sensors in the requestEvent() if it is fast enough
  // and just putting a delay (or not) in the loop. Otherwise, you can continuously
  // update your x,y recommendations and send them when requested.
  
  delay(1); // this will be fine tuned down the line
}

void requestEvent(){
  // Code to run when primary UNO requests data
  
  // Check your sensors
  // digitalRead... etc.
  
  // Send (x,y) recommendation
  Wire.write(x); // respond with (x, y) recommendation (or whatever you want)
  Wire.write(y); // primary must know how many bits to expect (i.e. let me know)
}

void receiveEvent(){
  // Code to run when primary UNO broadcasts data (x,y heading)
  x_joy = Wire.read(); // receive first joystick byte
  if(DEBUG) Serial.print(x_rec); // print the character (for debugging)
  
  y_joy = Wire.read();    // receive second joystick byte
  if(DEBUG) Serial.println(y_rec); // print the integer (for debugging)
}

