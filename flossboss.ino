/*
 * Code by Tom Dance (tom.dance@gmail.com)
 * Some code lifted from: https://github.com/BretStateham/28BYJ-48
 * Video here: https://channel9.msdn.com/Shows/themakershow/8
 */

#include "AccelStepper.h"
#define HALFSTEP 8
#define FULLSTEP 4

// for tracking internal state
bool flossOpen = false;
bool pasteRelock = false;
bool initialRotateDone = false;

// millis * seconds
long pasteUnlockWait = (1000L * 30);   // debug: 3 prod: 30 - time to wait between floss open and paste unlock (millis * seconds)
long pasteRelockWait = (1000L * 61200);   // debug: 5 prod: 61200 - time between paste unlock and relock (61200 = 17 hours delay) 
long serialReadDelay = (1000L * 5);   // debug: 1 prod: 5 - accelerometer read delay, so as not to overload it

// y axis position
int accelY;

// specify pin variables based on their colour
#define blue 2
#define pink 3
#define yellow 4
#define orange 5

// define steps for 180 degree rotation (eg. to unlock or lock)
// int targetPosition = 2048;  //2049 steps per rotation when wave or full stepping
int targetPosition = 2048;  //4096 steps per rotation when half stepping

// initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
// passing them as Blue, Yellow, Pink, Orange (coil ends order) not
// Blue, Pink, Yellow, Orange (firing order)
AccelStepper stepper(HALFSTEP, blue, yellow, pink, orange);

// the setup routine runs once when you press reset:
void setup() {
  
  // initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor

  // set the initial speed (read the AccelStepper docs on what "speed" means)
  stepper.setSpeed(700.0);
  
  // tell it how fast to accelerate
  stepper.setAcceleration(2000.0);
  
  // set a maximum speed it should not exceed 
  stepper.setMaxSpeed(1000.0);

  // delay on boot to be ready for initial 180 degree rotation
  delay(5000);
}

// the loop routine runs over and over again forever:
void loop() {

  // perform an initial 180 rotate to allow for relocking and/or correct positioning
  if (!initialRotateDone) {
    rotate180();
  }

  // read input on analog pin A1 (Y axis) to see if the floss side is closed
  if (!flossOpen && !pasteRelock && initialRotateDone) {
    Serial.println("Checking for rotation on Y axis...");
    delay(serialReadDelay);

    // read analog input on A1
    accelY = analogRead(A1);

    // Serial.println(accelY);
    
    // check the angle of the accelerometer
    if (!flossOpen && accelY < 280) {
      flossOpen = true;
      
      Serial.print("Floss side opened, waiting ");
      Serial.print(pasteUnlockWait / 1000);
      Serial.print(" seconds to unlock paste side...");      
      delay(pasteUnlockWait);
      Serial.println("unlocking now.");
    }        
  }

  if(flossOpen && !pasteRelock) {
    unlock();
  }

  if(pasteRelock) {
    lock();
  }  
}

/*
 * Functions
 */

// reset state of bools 
void resetState() {
  flossOpen = false;
  pasteRelock = false;
  initialRotateDone = true;
}

// unlock the paste side
void unlock() {

  // rotate 180 degrees (0 or home position due to initial 180 degree rotation on boot)
  stepper.moveTo(0);

  // check to see if the stepper has reached the target
  if(stepper.distanceToGo() != 0) {
    // move the stepper to its target pos
    stepper.run();
  }
  else {
    pasteRelock = true;
    Serial.print("Waiting ");
    Serial.print(pasteRelockWait / 1000);
    Serial.print(" seconds to relock...");
    delay(pasteRelockWait);
    Serial.println("relocking now.");
  }
}

// lock the paste side
void lock() {

  // rotate 180 degrees
  stepper.moveTo(targetPosition);

  // check to see if the stepper has reached the target
  if(stepper.distanceToGo() != 0) {
    // move the stepper to its target pos
    stepper.run();  
  }
  else {
    Serial.println("All done. Resetting values");
    resetState();
  }  
}

// basic 180 degree rotation that only runs once
void rotate180() {
  stepper.moveTo(targetPosition);
  if(stepper.distanceToGo() != 0) {
    stepper.run();  
  }
  else {
    Serial.println("Positioning 180 degree rotation done");
    initialRotateDone = true;  
  }
}

