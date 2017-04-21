//******************
// Basic Chair Joystick Control
// R. Kuc
// 1-22-17
// modified 3-22-17 by B. Fischer to include i2c
// ******************
// Connections
// Battery goes to B+ and B
// Motor goes to M+ and M-
// i2c bus on 3.5mm cable:
// SDA - tip
// SCL - ring 
// GND - sleeve
//
// i2c addresses
// Voice Recognition - 15
// Teleoperations - 14

#include <Wire.h>

#define DEBUG 1
#define FAULT_BRIDGE 3
#define HBRIDGE_FB_R 7 // CAT6(6)
#define HBRIDGE_FB_L 8 // CAT6(3)
#define R_PWM 9 // CAT6(5)
#define L_PWM 10 // CAT6(4)
#define L_COMM_PWM 6
#define R_COMM_PWM 11
// JS_XAXIS <-> Arduino A0
#define JS_XAXIS A0
// JS_YAXIS <-> Arduino A1
#define JS_YAXIS A1
// Scaler Potentiometer
#define SCALER A2
// Input Select Pin
#define AUTO_SELECT 2
#define TELEOP_SELECT 4
#define VOICE_SELECT 5
// 

// ******************
// I2C Addresses
#define A_SONAR 8
#define A_PROX 9
#define A_LIDARLITE 10
#define A_CAMERA 11
#define A_LIDAR 12
#define A_VOICE 15
#define A_TELEOP 14

// ******************
// Macros
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define B0UND(x,a,b) MAX(a, MIN(x, b))
// ******************
// Global Variables
int X; // X variable
int Y; // Y variable
long int xy; // xy receive variable
int data1, data2; // i2c data receive variables
int x_check, y_check; // X and Y obstacle checks
volatile int left_wheel;
volatile int right_wheel;
int left_wheel_old; // old lw variable
int right_wheel_old; // old rw variable
int minDriveLevel = 20;
boolean blockForward = false;
boolean blockBackward = false;
boolean blockLeft = false;
boolean blockRight = false;
boolean notAutomatic = true, notTeleop = true, notVoice = true;
// for i2c search
int present[16];
int error;

// ******************
// Function Prototypes
void getJoystick(void);
void broadcastHeading(void);
void getSensors(void);
void calculateHeading(void);
void drive(int, int, boolean);
void checkSensor(int);
void autoMode(void);
void i2c(void);

// ******************

void setup() {
  // initialize serial
  Serial.begin(115200);
  //Serial.setTimeout(500); // make parseInt faster to timeout

  /*
  Wire.begin();         // join i2c bus as master
  
  for (int i = 8; i<16; i++){
    Wire.beginTransmission(i);
    if (Wire.endTransmission()==0) present[i] = 1; // sensor at address i is present
    else present[i] = 0;
    delay(5);
  }
  */

  // scooter is not moving
  X = 0;
  Y = 0;

  // initialize SELECT pins
  pinMode(AUTO_SELECT, INPUT_PULLUP);
  pinMode(VOICE_SELECT, INPUT_PULLUP);
  pinMode(TELEOP_SELECT, INPUT_PULLUP);

  // initialize connection to Matlab
  Serial.println("UNO Ready");
  
  // initialize H-Bridge enable pins
  pinMode(HBRIDGE_FB_R, OUTPUT);
  digitalWrite(HBRIDGE_FB_R, HIGH);
  pinMode(HBRIDGE_FB_L, OUTPUT);
  digitalWrite(HBRIDGE_FB_L, HIGH);

  // initialize H-Bridge PWM pins
  pinMode(R_PWM, OUTPUT);
  digitalWrite(R_PWM, LOW);
  pinMode(L_PWM, OUTPUT);
  digitalWrite(L_PWM, LOW);

  // initialize comm PWM pins
  pinMode(R_COMM_PWM, OUTPUT);
  digitalWrite(R_COMM_PWM, LOW);
  pinMode(L_COMM_PWM, OUTPUT);
  digitalWrite(L_COMM_PWM, LOW);

  // speed up pwm to get rid of humming
  // we can do this with the pololus because the max pwm frequency is 40kHz
  // this will set the frequency to 31.374 kHz
  TCCR1B = TCCR1B & B11111000 | B00000001;
}

// ******************
// Main Loop
void loop() {
  delay(100);
  notAutomatic = digitalRead(AUTO_SELECT);
  notVoice = digitalRead(VOICE_SELECT);
  notTeleop = digitalRead(TELEOP_SELECT);
  
  if (notAutomatic){
    if(notVoice && notTeleop){
      getJoystick(); // Drive with Joystick
      if(DEBUG) Serial.println("Joystick");
    }
    else if(notTeleop){
      i2c(A_VOICE); // take commands from voice recognition
      if(DEBUG) Serial.println("Voice Recognition");
    }
    else if(notVoice){
      i2c(A_TELEOP); // take commands from teleoperation
      if(DEBUG) Serial.println("Teleoperation");
    }
    else{
      X = 0;
      Y = 0;
      Serial.println("Error - No Input");
    }
  }
  else{
    autoMode();
    if(DEBUG) Serial.println("Auto Mode");
  }
 
  calculateHeading();
  // broadcastHeading();
  drive(right_wheel, left_wheel);
  
} // end loop

// ******************
// Functions

void i2c(int address){
  Wire.requestFrom(address, 2); // request two bytes (x and y) bytes from address
  delay(10);
  if(Wire.available()){
    X = Wire.read();
    Y = Wire.read();
    Y = -Y; // flip Y input
  }
}

void autoMode(){
  if(Serial.available()){
    xy = Serial.parseInt(); // send value as xxxyyy where x = (1,255); y = (1,255)
  }
  if(xy){ // xy is non-zero, update values
    X = xy/1000; // yyy will never be >255, so no need to typecast as float and round down
    Y = xy%1000; 
    Y = -Y;      // flip Y input
  }
  
  // debugging code:
  /*if(DEBUG){
    Serial.print("Auto Mode:");
    Serial.print(X);
    Serial.print(", ");
    Serial.println(Y);*/
}


void broadcastHeading(){  // function to broadcast (x,y) heading to secondary UNOs
    // ensure that the following line is in the secondary's setup code:
    // TWAR = (MY_ADDRESS << 1) | 1;  // enable broadcasts to be received
    
    Wire.beginTransmission(0); // broadcast to all
    Wire.write(X);
    Wire.write(Y);
    error = Wire.endTransmission();
}

void getJoystick() {
  // X axis scaled for 8 bit OUTPUT
  X = (analogRead(JS_XAXIS) - 512) / 4;
  // Y axis scaled for 8 bit OUTPUT
  Y = (analogRead(JS_YAXIS) - 512) / 4;
  // flip Y input
  Y = -Y;
  // debugging code:
  /*if(DEBUG){
    Serial.print("Joystick:");
    Serial.print(X);
    Serial.print(", ");
    Serial.println(Y);
  }*/
}

void calculateHeading(){
    // calculate appropriate speeds
  double magnitude = MAX(255, sqrt(abs(X * X) + abs(Y * Y)));
  double Forward_speed = Y / magnitude;
  double turn_speed = X / magnitude;
  left_wheel = (int) 127 * (1 + Forward_speed + turn_speed);
  right_wheel = (int) 127 * (1 + Forward_speed - turn_speed);

  // bind magnitude
  left_wheel = B0UND(left_wheel, 0, 255);
  right_wheel = B0UND(right_wheel, 0, 255);

  // calculate outputs
  left_wheel -= 127;
  right_wheel -= 127;
  left_wheel *= 4;
  right_wheel *= 4;

  // bind values to appropriate PWM range
  left_wheel = B0UND(left_wheel, -255, 255);
  right_wheel = B0UND(right_wheel, -255, 255);

  // check for noise
  if (abs(left_wheel) < minDriveLevel)
    left_wheel = 0;
  if (abs(right_wheel) < minDriveLevel)
    right_wheel = 0;

  // scale OUTPUT according to Potentiometer position
  int scalingFactor = analogRead(SCALER);
  Serial.println(scalingFactor);
  double driveStraightScaler = (1003.0 / 1023.0);
  double tempRight = (double) (right_wheel) * (scalingFactor / 1023.0) *
                     driveStraightScaler;
  double tempLeft = (double) (left_wheel) * (scalingFactor / 1023.0);
  // store final values
  left_wheel = (int) tempRight;
  right_wheel = (int) tempLeft;
  // send speeds to terminal
  Serial.print("Left: ");
  Serial.print(left_wheel);
  Serial. print (" Right: ");
  Serial.println(right_wheel);
}

void drive(int right, int left) {
  // send Joystick speed values to Mega
  analogWrite(L_COMM_PWM, abs(left));
  analogWrite(R_COMM_PWM, abs(right));
  // LOW to H-Bridge is Forward, HIGH to H-Bridge is Backward

// Check if JS tells wheelchair to move Forward
   if (right <= 0 && left <= 0) {
    if (!blockForward) {
      digitalWrite(HBRIDGE_FB_R, LOW);
      analogWrite(R_PWM, abs(right));
      digitalWrite(HBRIDGE_FB_L, LOW);
      analogWrite(L_PWM, abs(left));
    }
    else {
      digitalWrite(HBRIDGE_FB_R, LOW);
      analogWrite(R_PWM, 0);
      digitalWrite(HBRIDGE_FB_L, LOW);
      analogWrite(L_PWM, 0);
    }
  }
  // Check if JS tells wheelchair to turn left
  else if (right <= 0 && left > 0) {
    if (!blockLeft) {
      digitalWrite(HBRIDGE_FB_R, LOW);
      analogWrite(R_PWM, abs(right));
      digitalWrite(HBRIDGE_FB_L, HIGH);
      analogWrite(L_PWM, abs(left));
    }
    else {
      digitalWrite(HBRIDGE_FB_R, LOW);
      analogWrite(R_PWM, 0);
      digitalWrite(HBRIDGE_FB_L, HIGH);
      analogWrite(L_PWM, 0);
    }
  }
  // Check if JS tells wheelchair to turn right
  else if (right > 0 && left <= 0) {
    if (!blockRight) {
      digitalWrite(HBRIDGE_FB_R, HIGH);
      analogWrite(R_PWM, abs(right));
      digitalWrite(HBRIDGE_FB_L, LOW);
      analogWrite(L_PWM, abs(left));
    }
    else {
      digitalWrite(HBRIDGE_FB_R, HIGH);
      analogWrite(R_PWM, 0);
      digitalWrite(HBRIDGE_FB_L, LOW);
      analogWrite(L_PWM, 0);
    }
  }
  // Check if JS tells wheelchair to move backward
  else if (right > 0 && left > 0) {
    if (!blockBackward) {
      digitalWrite(HBRIDGE_FB_R, HIGH);
      analogWrite(R_PWM, abs(right));
      digitalWrite(HBRIDGE_FB_L, HIGH);
      analogWrite(L_PWM, abs(left));
    }
    else {
      digitalWrite(HBRIDGE_FB_R, HIGH);
      analogWrite(R_PWM, 0);
      digitalWrite(HBRIDGE_FB_L, HIGH);
      analogWrite(L_PWM, 0);
    }
  }
}

