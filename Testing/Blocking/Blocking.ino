// Blocking.pde
// -*- mode: C++ -*-
//
// Shows how to use the blocking call runToNewPosition
// Which sets a new target position and then waits until the stepper has 
// achieved it.
//
// Copyright (C) 2009 Mike McCauley
// $Id: Blocking.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

//Stepper defines
#define E0_STEP 26 //Digital 26
#define E0_DIR 28
#define E0_EN 24

#define E1_STEP 36
#define E1_DIR 34
#define E1_EN 30

#define X_STEP 54 //A0
#define X_DIR 55 //A1
#define X_EN 38

#define Y_STEP 60 //A6
#define Y_DIR 61 //A7
#define Y_EN 56 //A2

#define Z_STEP 46
#define Z_DIR 48
#define Z_EN 62 //A8?

//endstop defines
//xmin,xmax,ymin,ymax,zmin,zmax
#define X_MIN           3
#define X_MAX           2

#define Y_MIN          14
#define Y_MAX          15

#define Z_MIN          18
#define Z_MAX          19

// Define a stepper and the pins it will use
//AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper Vstepper(AccelStepper::DRIVER, Z_STEP, Z_DIR);
AccelStepper Hstepper(AccelStepper::DRIVER, Y_STEP, Y_DIR);
//AccelStepper stepper(1,Z_STEP,Z_DIR);

long initial_homing=0;

void setup()
{  
    Serial.begin(9600);
    setStepperDefaults();

    pinMode(Z_STEP, OUTPUT);
    pinMode(Z_DIR, OUTPUT);
    pinMode(Z_EN, OUTPUT);
    digitalWrite(Z_EN, LOW);

    pinMode(Y_STEP, OUTPUT);
    pinMode(Y_DIR, OUTPUT);
    pinMode(Y_EN, OUTPUT);
    digitalWrite(Y_EN, LOW);
    Serial.println("Accel test");

    homeStepper(Vstepper,Z_MIN);
    Serial.print("60: ");
    Serial.println(angleToSteps(60));
    Serial.print("10: ");
    Serial.println(angleToSteps(10));
    Serial.print("360: ");
    Serial.println(angleToSteps(360));
}

void loop()
{    
    //up in steps is CCW, down in steps is CW
//    stepper.runToNewPosition(0);
//    delay(1000);
//    stepper.runToNewPosition(-200);
//    delay(1000);
//    stepper.runToNewPosition(-400);
//    delay(1000);
}

void homeStepper(AccelStepper myStepper, int EndStopPin){
//  https://www.brainy-bits.com/setting-stepper-motors-home-position-using-accelstepper/
  myStepper.setMaxSpeed(100.0); //slow the stepper down
  myStepper.setAcceleration(100.0); //slow the acceleration down
  delay(5); //wait for driver to wake up
  Serial.println("Start Homing...");
  while (digitalRead(EndStopPin)){ //move CW until switch is pressed
    myStepper.moveTo(initial_homing);
    myStepper.run();
    initial_homing--;
    delay(5);
  }
  myStepper.setCurrentPosition(0); //set current position as zero
  myStepper.setMaxSpeed(50.0); //slow the stepper down
  myStepper.setAcceleration(50.0); //slow the acceleration down
  initial_homing=0;

  while (!digitalRead(EndStopPin)){ //move CCW slowly until switch is released
    myStepper.moveTo(initial_homing);
    myStepper.run();
    initial_homing++;
    delay(5);
  }
  myStepper.setCurrentPosition(0);
  Serial.println("Homing Complete!");
  setStepperDefaults();
}

void setStepperDefaults(){
    Vstepper.setMaxSpeed(200.0); //could reasonably up these to 1000/1000 but lets start with 200/100 for now
    Vstepper.setAcceleration(100.0);
    Hstepper.setMaxSpeed(200.0);
    Hstepper.setAcceleration(100.0);
}

int angleToSteps(int angle){
  #define gearTeeth 540.0 //number of teeth on the device large gear (assuming gear was full in case of arch)
  #define stepperTeeth 13.0 //number of teeth on the stepper's gear
  #define stepper360 200.0 //number of steps for a full stepper rotation
  int steps = 0;
  steps = ( (gearTeeth/stepperTeeth)*(stepper360/360)*angle);
  //Serial.print("steps: ");
  //Serial.println(steps);
  return steps;
}

int stepsToAngle(int steps){
  
}

void rolloverAngle(int currentAngle, int futureAngle){
  //divisible by 360 degrees
  //int range from -32,768 to 32,767
  //if 360 degrees is 8307 steps
  //max rotations is 3.94
  //if overshooting 0, move to 360 and set a new 0 point
  //better than "unwinding" to 0 and starting over or overflowing steps int

  //long range from -2,147,483,648 to 2,147,483,647
  //max rotations 258,514.94
  //if resetting horizontal steps to 0 before "start" every new run this works

  //after last horizontal picture, go to 360 degrees, set it as new 0 point
  //then move vertical height, etc.
}
