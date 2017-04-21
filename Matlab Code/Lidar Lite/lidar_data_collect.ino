/*------------------------------------------------------------------------------
  R'mani Haulcy
  EENG 481
  Goal: Collect distance information as servo motor moves back and forth (create a radar-like map)

  LIDARLite Connections:
  LIDAR-Lite 5 Vdc (red) to Arduino 5v
  LIDAR-Lite I2C SCL (green) to Arduino SCL A5
  LIDAR-Lite I2C SDA (blue) to Arduino SDA A4
  LIDAR-Lite Ground (black) to Arduino GND

  Servo Motor Connections:
  red --- 5V
  black -- GND
  white -- signal (pin 6)

  Horizontal Range: 0-180
  Vertical Range: 0-90
*/
#include <math.h>  // c math library for sin() function
#include <Wire.h>
#include <LIDARLite.h>
LIDARLite myLidarLite;
#include <Servo.h>
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
int dist = 0; //variable to store the distance measured by the sensor
byte sineTab[1000]; // table of byte values
float arg, x, pi = 4 * atan(1), x_obs,y_obs,x_chair; // defines pi to the precision of cpu

void setup()
{
  Serial.begin(115200); // Initialize serial connection to display distance readings
  myservo_h.attach(6);  // attaches the servo on pin 9 to the servo object
  /*
    begin(int configuration, bool fasti2c, char lidarliteAddress)
    Starts the sensor and I2C.
    Parameters
    ----------------------------------------------------------------------------
    configuration: Default 0. Selects one of several preset configurations.
    fasti2c: Default 100 kHz. I2C base frequency.
      If true I2C frequency is set to 400kHz.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz

  /*
    configure(int configuration, char lidarliteAddress)
    Selects one of several preset configurations.
    Parameters
    ----------------------------------------------------------------------------
    configuration:  Default 0.
      0: Default mode, balanced performance.
      1: Short range, high speed. Uses 0x1d maximum acquisition count.
      2: Default range, higher speed short range. Turns on quick termination
          detection for faster measurements at short range (with decreased
          accuracy)
      3: Maximum range. Uses 0xff maximum acquisition count.
      4: High sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for high sensitivity and noise.
      5: Low sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for low sensitivity and noise.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.configure(0); // Change this number to try out alternate configurations

// generate sine value table
  for (int i = 0; i < 1000; i++) {
    arg = 2.*pi * double(i) / 1000.; // arg has 1 period in 1000 points
    x = 1 + sin(arg); // makes x in [0,2)
    sineTab[i] = floor(30.*x) + 60; // forms int [60,120] 
}
}

void loop()
{
  /*
    distance(bool biasCorrection, char lidarliteAddress)
    Take a distance measurement and read the result.
    Parameters
    ----------------------------------------------------------------------------
    biasCorrection: Default true. Take aquisition with receiver bias
      correction. If set to false measurements will be faster. Receiver bias
      correction must be performed periodically. (e.g. 1 out of every 100
      readings).
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */

//Sinusoidal servo motion
for (int i = 0; i < 1001; i++){
  pos = sineTab[i];
  myservo_h.write(pos); // tell servo to go to position in variable 'pos'
  //first column in serial monitor is distance, second column is angle
  dist = myLidarLite.distance();
  Serial.print(dist);
  Serial.print("\t");
  Serial.println(pos);

//obstacle avoidance
if(dist < 90 && pos >= 60 && pos <= 120){//decide on how close you want the obstacle to be before reacting (currently ~3FT)
  //x and y position of the obstacle from the sensor 
  x_obs = dist*cos(pos*(180/PI)); //convert degrees to radians
  y_obs = dist*sin(pos*(180/PI)); //convert degrees to radians
  if(x_obs < 0){ //if there's an obstacle to the left of the chair
    x_chair = 64; //move chair to the right
  }
  else if(x_obs > 0){ //if there's an obstacle to the right of the chair
    x_chair = -64; //move chair to the left
  }
  else {
    x_chair = -64; // if there is an obstacle straight ahead move chair to the left as a default 
  } 
}
}

//non-sinusoidal motion
//for (pos = 60; pos <= 120; pos += 1) { // servo goes from 60 degrees to 120 degrees (90 degrees is straight ahead)
//                                          // in steps of 1 degree
//  myservo.write(pos); // tell servo to go to position in variable 'pos'
//  //first column in serial monitor is x, second column is y
//  dist = myLidarLite.distance();
//  Serial.print(dist);
//  Serial.print("\t");
//  Serial.println(pos);
//  delay(15); // waits 15ms for the servo to reach the position
//    
//  }
//  for (pos = 120; pos >= 60; pos -= 1) { // goes from 120 degrees to 60 degrees
//                                          // in steps of 1 degree
//  myservo.write(pos);              // tell servo to go to position in variable 'pos'
//  dist = myLidarLite.distance();
//  Serial.print(dist);
//  Serial.print("\t");
//  Serial.println(pos);
//  delay(15); // waits 15ms for the servo to reach the position
//  }
} 
