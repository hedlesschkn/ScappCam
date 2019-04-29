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
#define E0_STEP D26
#define E0_DIR D28
#define E0_EN D24
#define E1_STEP D36
#define E1_DIR D34
#define E1_EN D30
#define X_STEP A0
#define X_DIR A1
#define X_EN D38
#define Y_STEP A6
#define Y_DIR A7
#define Y_EN A2
#define Z_STEP D46
#define Z_DIR D48
#define Z_EN A8

//endstop defines
#define end_Xmin D3
#define end_Xmax D2
#define end_Ymin D14
#define end_Ymax D15
#define end_Zmin D18
#define end_Zmax D19

//thermister defines
#define T0 A13
#define T1 A14
#define T2 A15

//Servo defines
#define Servo1 D11
#define Servo2 D6
#define Servo3 D5
#define Servo4 D4

//LCD defines(16x2 or 20x4 w/ LiquidCrystal library)
//https://forum.arduino.cc/index.php?topic=448001.0
//#define BEEPER 33           // Beeper and is Connected into GADGETS3D shield MEGA_18BEEPER
//#define LCD_PINS_RS 16      // LCD control and is connected into GADGETS3D  shield LCDRS
//#define LCD_PINS_ENABLE 17  // LCD enable pin and is connected into GADGETS3D shield LCDE
//#define LCD_PINS_D4 23      // LCD signal pin, connected to Gadgets3D shield LCD4
//#define LCD_PINS_D5 25      // LCD signal pin, connected to Gadgets3D shield LCD5
//#define LCD_PINS_D6 27      // LCD signal pin, connected to Gadgets3D shield LCD6
//#define LCD_PINS_D7 29      // LCD signal pin, connected to Gadgets3D shield LCD7
//#define BTN_EN1 37          // Encoder left direction, connected to Gadgets3D shield S_E1
//#define BTN_EN2 35          // Encoder right direction, connected to Gadgets3D shield S_E2
//#define BTN_ENC 31          // Encoder Click, connected to Gadgets3D shield S_EC
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);

//LCD defines (12864 RepRapDiscount Full Graphic w/ u8glib library)
//https://reprap.org/wiki/RepRapDiscount_Full_Graphic_Smart_Controller
//https://reprap.org/mediawiki/images/7/79/LCD_connect_SCHDOC.pdf
//https://reprap.org/forum/read.php?1,675863
//https://github.com/ellensp/rrd-glcd-tester/blob/master/rrd-glcd-test.ino
//use u8g2 instead of u8glib
#include <SPI.h>
#include <U8glib.h>
#include <SD.h>


void setup() {

}

void loop() {

}
