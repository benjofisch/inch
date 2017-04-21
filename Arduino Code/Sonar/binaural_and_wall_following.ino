// Binaural Sonar for Determining if an object is to the
// left, middle or right, and approximate range of the object
// uses voltage across cap in 555 timer circuit to yield range

// A. Arkebauer
// EENG 481
// 2/27/17

#define INIT 6
#define LIN 2 // left input
#define BLANK 4
#define RIN 5 // right input

// LEDs
#define LED_L 11
//#define LED_M 12
//#define LED_R 13
#define LED_M 9
#define LED_R 10

// Timer
#define TIMER A0
#define TRIGGER 8

// External interrupt - high when either LIN or RIN rises
#define COMPARE 3

// Wall following ECHO signals
#define wallLeft 12
#define wallRight 13

volatile int range; // output
volatile float avg; // output

int right;

int numPolls = 10;
volatile int poll[10]; // used to average side readings to determine if object on left/right/middle
volatile int pollInd = 0;

volatile int OUTPUT_FLAG = 0; // set to 1 to indicate output is ready to be sent

volatile int objPos = 0; // 0: left; 1: middle; 2: right

void setup() {
  pinMode(INIT, OUTPUT);

  pinMode(LIN, INPUT);
  digitalWrite(LIN, 1); // pull-up R
  pinMode(RIN, INPUT);
  digitalWrite(RIN, 1); // pull-up R

  digitalWrite(INIT, 0);
  pinMode(BLANK, OUTPUT);
  digitalWrite(BLANK, 0);

  pinMode(LED_L, OUTPUT);
  digitalWrite(LED_L, 0);
  pinMode(LED_M, OUTPUT);
  digitalWrite(LED_M, 0);
  pinMode(LED_R, OUTPUT);
  digitalWrite(LED_R, 0);

  Serial.begin(115200);

  pinMode(TIMER, INPUT); // connect to cap on 555 timer
  pinMode(TRIGGER, OUTPUT); // trigger to begin timing
  digitalWrite(TRIGGER, HIGH); // low signal triggers timing - connect to pin 2 of 555 timer


  pinMode(wallLeft, INPUT);
  pinMode(wallRight, INPUT);

  attachInterrupt(digitalPinToInterrupt(COMPARE), light, RISING);
}

int tStart, leftWallDist, rightWallDist;

void loop() {
  digitalWrite(TRIGGER, HIGH);
  digitalWrite(INIT, 1);
  digitalWrite(TRIGGER, LOW);

  // wall following
  tStart = micros();
  int wallRightRead = 0;
  int wallLeftRead = 0;
  while ((!wallRightRead || !wallLeftRead)) {
    //    Serial.print("wall right: "); Serial.println(digitalRead(wallRight));
    //    Serial.print("wall left: "); Serial.println(digitalRead(wallLeft));
    if (digitalRead(wallRight) && !wallRightRead) {
      rightWallDist = micros() - tStart;
      wallRightRead = 1;
    }
    if (digitalRead(wallLeft) && !wallLeftRead) {
      leftWallDist = micros() - tStart;
      wallLeftRead = 1;
    }

    //        if (micros()-tStart > 25000){
    //      if (!wallRightRead){
    //        rightWallDist = 25000;
    //            wallRightRead = 1;
    //      }
    //      if (!wallLeftRead){
    //        leftWallDist = 25000;
    //            wallLeftRead = 1;
    //      }
    //        }

  }

  delay(20);

  // at this point, interrupt will have executed

  // above 25000 is meaningless - wall is very far away
  if (rightWallDist < 0 || rightWallDist > 25000) {
    rightWallDist = 25000;
  }
  if (leftWallDist < 0 || leftWallDist > 25000) {
    leftWallDist = 25000;
  }

  Serial.print("Right Wall: "); Serial.print(rightWallDist);
  Serial.print(" Left Wall: "); Serial.print(leftWallDist);

  digitalWrite(INIT, 0);
  delay(20);

  /*
    at this point, avg (left/right/middle indication)
    and range (larger for farther objects) are ready to be output
  */

  //  objPos; // 0: left; 1: middle; 2: right
  //  range/4; // higher indicates farther away - divide by 4 to get a byte

  Serial.print(" Position: "); Serial.print(objPos);
  Serial.print(" Range: "); Serial.println(range / 4);

}




void light() { // triggered when either LIN or RIN rises
  //  delay(1); // by adding a small delay, can increase the size of the "middle" range
  if (digitalRead(RIN)) {
    if (digitalRead(LIN)) {
      //middle
      digitalWrite(LED_L, LOW);
      digitalWrite(LED_R, LOW);
      digitalWrite(LED_M, HIGH);
      objPos = 1;
    } else {
      //right
      digitalWrite(LED_L, LOW);
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_M, LOW);
      objPos = 2;
    }
  } else {
    //left
    digitalWrite(LED_L, HIGH);
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_M, LOW);
    objPos = 0;
  }

  range = analogRead(TIMER);
  //  Serial.println(range);
}

