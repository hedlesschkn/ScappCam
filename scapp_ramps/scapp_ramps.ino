/**
 * Code by Tim Hebert
 * Brandeis Automation Lab
 * 4-22-2019
 * 
 * ScappCam code for Arduino Mega 2560 with RAMPS 1.4
 * wiring diagram:
 * https://robotdyn.com/pub/media/0G-00005255==RAMPS1_4-Shiled/DOCS/PINOUT==0G-00005255==RAMPS1.4-Shiled.jpg
 * https://reprap.org/forum/thumbcache/e2e/904/ab8/8d0/241/ad4/1f0/c64/0c3/689/24_800x400.png
 * https://user-images.githubusercontent.com/26955383/34589604-dbd41b72-f16e-11e7-99a5-c91d116a0bd9.png
 * 
 * Stepstick rev 0.1
 * A4988 microstepping driver
 * 4988ET 859H
 * https://www.pololu.com/product/1182
 */

//Stepper defines
#define E0_STEP 26 //Digital 26
#define E0_DIR 28
#define E0_EN 24
#define E1_STEP 36
#define E1_DIR 34
#define E1_EN 30
#define X_STEP A0
#define X_DIR A1
#define X_EN 38
#define Y_STEP A6
#define Y_DIR A7
#define Y_EN A2
#define Z_STEP 46
#define Z_DIR 48
#define Z_EN A8

//endstop defines
#define end_Xmin 3
#define end_Xmax 2
#define end_Ymin 14
#define end_Ymax 15
#define end_Zmin 18
#define end_Zmax 19

//thermister defines
#define T0 A13
#define T1 A14
#define T2 A15

//Servo defines
#define Servo1 11
#define Servo2 6
#define Servo3 5
#define Servo4 4

//LCD defines (12864 RepRapDiscount Full Graphic w/ u8glib library)
//https://reprap.org/wiki/RepRapDiscount_Full_Graphic_Smart_Controller
//https://reprap.org/mediawiki/images/7/79/LCD_connect_SCHDOC.pdf
//https://reprap.org/forum/read.php?1,675863
//https://github.com/ellensp/rrd-glcd-tester/blob/master/rrd-glcd-test.ino
//use u8g2 instead of u8glib
#include <SPI.h>
#include <U8glib.h> //Tools > Manage Libraries... > "U8glib" by Oliver > Install
#include <SD.h>

#define DOGLCD_CS       16
#define DOGLCD_MOSI     17
#define DOGLCD_SCK      23
#define BTN_EN1         31
#define BTN_EN2         33
#define BTN_ENC         35
#define SD_DETECT_PIN   49
#define SDSS            53
#define BEEPER_PIN      37
#define KILL_PIN        41

Sd2Card card;
SdVolume volume;

//steper example
//https://howtomechatronics.com/tutorials/arduino/how-to-control-stepper-motor-with-a4988-driver-and-arduino/
//https://forum.arduino.cc/index.php?topic=377364.0
//  you need to use millis() or micros() to control the timing without blocking. The delay() function blocks the Arduino until it finishes.
//  will i need to use interrupts to trigger steps to handle multiple motors simultaniously?

//SCAPPCAM Parameters
#define base_teeth    540
#define pinion_teeth  13
#define stepper_360   200 //number of steps for full rotation


void setup() {
  //stepper pins
  pinMode(E0_STEP,OUTPUT); 
  pinMode(E0_DIR,OUTPUT);
  pinMode(E0_EN,OUTPUT);
  digitalWrite(E0_EN,LOW); //ENABLE pin is used for turning on or off the FET outputs. So a logic high will keep the outputs disabled.

  //LCD pins
  pinMode(KILL_PIN, INPUT);             // Set KILL_PIN as an unput
  digitalWrite(KILL_PIN, HIGH);         // turn on pullup resistors
  pinMode(BTN_EN1, INPUT);              // Set BTN_EN1 as an input, half of the encoder
  digitalWrite(BTN_EN1, HIGH);          // turn on pullup resistors
  pinMode(BTN_EN2, INPUT);              // Set BTN_EN2 as an input, second half of the encoder
  digitalWrite(BTN_EN2, HIGH);          // turn on pullup resistors
  pinMode(BTN_ENC, INPUT);              // Set BTN_ENC as an input, encoder button
  digitalWrite(BTN_ENC, HIGH);          // turn on pullup resistors
  u8g.setFont(u8g_font_helvR08);        //Set the font for the display
  u8g.setColorIndex(1);                 // Instructs the display to draw with a pixel on. 
}

void loop() {
//can also try the 'AccelStepper' library
//digitalWrite(E0_DIR,HIGH); // Enables the motor to move in a particular direction
//  // Makes 200 pulses for making one full cycle rotation
//  for(int x = 0; x < 200; x++) {
//    digitalWrite(E0_STEP,HIGH); 
//    delayMicroseconds(400); //longer delay is slower motor speed, shorter is faster. shorter also loses torque
//    digitalWrite(E0_STEP,LOW); 
//    delayMicroseconds(400); 
//  }
//  delay(1000); // One second delay
//  
//  digitalWrite(E0_DIR,LOW); //Changes the rotations direction
//  // Makes 400 pulses for making two full cycle rotation
//  for(int x = 0; x < 400; x++) {
//    digitalWrite(E0_STEP,HIGH);
//    delayMicroseconds(1500);
//    digitalWrite(E0_STEP,LOW);
//    delayMicroseconds(1500);
//  }
//  delay(1000);

//menu example
//https://arduino.stackexchange.com/questions/42897/menu-with-switch-case-not-behaving-correctly


  /*
   * display the menu
   * SCAPP CAM title
   * Vertical Height (current height in degrees)
   * Number of pictures (angle between pics)
   * wait time (time in ms)
   * Go!
   * show current/total pictures
   * show % completion
   * 
   */
  
}
