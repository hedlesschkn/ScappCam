/* TODO
 *  done - test accelstepper by mikem
 *  done - may need to accel slower or lower max speed from defaults given weight and drag
 *    seems like plenty of torque
 *    
 *  done - build calibration of vertical via end stop
 *    measure to check location for accuracy, might need to approach twice, 2nd time slower like 3dp calibration
 *    
 *  manually move steppers with manual mode
 *  calculate each picture location in steps
 *  done - set speed and accel, use blocking runToPosition to get to location
 *  test TRS focus and shutter command
 *    delay set pre picture wait time
 *    send picture command
 *    delay set post picture wait time
 *  go to next location, until done with horizontal
 *  calculate if continuing CW or unwinding ccw is closer, reset horizontal to 0 or 360 and call it new 0 step position
 *    move vertical height and loop again until complete.
 * 
 * might need to add 'shutter' menu
 * -wait time before picture (machine settle time, vhigh for macro)
 * -wait time after picture (long exposure)
 * -autofocus = ON/OFF
 * -BACK
 */

 //WIRING
 //vertical uses Z axis
 //horizontal uses Y axis
 //vertical endstop plugs into Z_min (xmin,xmax,ymin,ymax,zmin,zmax)

#include <SPI.h>
#include <RotaryEncoder.h> //ManageLibraries > RotaryEncoder by Matthias Hertel
//#include "U8g2lib.h" //ManageLibraries > U8g2
#include <U8glib.h>
#include <Wire.h>
#include <AccelStepper.h>

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
//interupt pins mega 2560 = 2, 3, 18, 19, 20, 21

//U8GLIB_SSD1351_128X128_332 u8g(8, 9, 7);
// Screen
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g(DOGLCD_SCK, DOGLCD_MOSI, DOGLCD_CS);
U8GLIB_ST7920_128X64_1X u8g(DOGLCD_SCK, DOGLCD_MOSI, DOGLCD_CS);

uint8_t screen = 0; //start on splash screen
uint8_t menu_current = 0;
uint8_t menu_max;

RotaryEncoder encoder(31, 33);
int newPos;
int cw,ccw;

int newOk = 0;
int okPressed = 0;
int okReleased = 0;

int selected = 0;

bool estop = LOW; //low is off, high is on
int turn;

char vAngleStr[16];   //Char array to store Vertical Angle as a string
char hAngleStr[16];   //Char array to store Horizontal Angle as a string
int vAngleInt = 0;    //manually set vertical angle number
int hAngleInt = 0;    //manually set horizontal angle number

char vPicStr[16];   //Char array to store Vertical Pictures as a string 
char hPicStr[16];   //Char array to store Horizontal Pictures as a string
int vPicInt = 0;    //number of vertical pics
int hPicInt = 30;    //number of horizontal pics

//camera variables
const int prePicTime = 1000;      //machine settle time before taking pic
const int shutterLength = 500;     //time to hold the shutter button down
const int focusLength = 1500;     //time to hold the focus button down

char afterPicWaitStr[16];   //Char array to store post pic delay 
int afterPicWaitInt = 1000;    //number in ms for post pic delay
bool autoFocus = LOW;

//servo defines
#define servo1 11
#define servo2 6
#define servo3 5
#define servo4 4
const int focusPin = servo1;
const int shutterPin = servo2;


//https://forum.arduino.cc/index.php?topic=151669.0

const uint8_t scappcam_bitmap[] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 
  0x1f, 0xff, 0xe3, 0xff, 
  0x7f, 0xff, 0xef, 0xff, 
  0x12, 0x22, 0x2e, 0x20, 
  0xd6, 0xaa, 0xae, 0xaa, 
  0x12, 0x22, 0x22, 0x2e, 
  0xff, 0xae, 0xff, 0xbf, 
  0xff, 0xee, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
  0xff, 0xf0, 0x0f, 0xff, 
  0xff, 0x80, 0x01, 0xff, 
  0xfe, 0x00, 0x00, 0x7f, 
  0xfc, 0x0f, 0xf0, 0x1f, 
  0xf0, 0x3f, 0xfc, 0x0f, 
  0xe0, 0xff, 0xff, 0x07, 
  0xe1, 0xff, 0xff, 0x87, 
  0xc3, 0xff, 0xff, 0xc3, 
  0xc7, 0xc0, 0x03, 0xe3, 
  0x84, 0x00, 0x00, 0x21, 
  0x80, 0x1f, 0xf8, 0x01, 
  0x83, 0xff, 0xff, 0xc1, 
  0x87, 0xff, 0xff, 0xe1, 
  0x80, 0x7f, 0xfe, 0x01, 
  0x80, 0x00, 0x00, 0x01, 
  0xc0, 0x00, 0x00, 0x03, 
  0xf0, 0x00, 0x00, 0x0f, 
  0xfb, 0x80, 0x01, 0xdf, 
  0xfb, 0xff, 0x7f, 0xdf, 
  0xff, 0xff, 0x7f, 0xff, 
  0xff, 0xff, 0x7f, 0xff, 
  0xff, 0xff, 0xff, 0xff, 
};

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

//other RAMPS defines
#define LED_PIN            13
#define FAN_PIN            9
#define HEATER_0_PIN       10
#define HEATER_1_PIN       8
#define TEMP_0_PIN         13   
#define TEMP_1_PIN         14
//use heated bed or hot end or fan as light controller?

// Define a stepper and the pins it will use
//AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper Vstepper(AccelStepper::DRIVER, X_STEP, X_DIR); //vertical uses X axis
AccelStepper Hstepper(AccelStepper::DRIVER, Z_STEP, Z_DIR); //horizontal uses Z axis because double output
//AccelStepper stepper(1,Z_STEP,Z_DIR);

long initial_homing=0;

void setup() {    
    Serial.begin(9600);
    Serial.println("ScappCam");
    u8g.begin(); // Added
    u8g.setFont(u8g_font_unifont); //10 pixel height
//  u8g.setDefaultForegroundColor();

    pinMode(KILL_PIN, INPUT);
    digitalWrite(KILL_PIN, HIGH);         // turn on pullup resistors
    
    pinMode(BTN_ENC, INPUT);              // Set BTN_ENC as an unput, encoder button
    digitalWrite(BTN_ENC, HIGH);          // turn on pullup resistors
    

    pinMode(focusPin,OUTPUT);
    pinMode(shutterPin,OUTPUT);
    
    //splash screen calibration
    screen = 0; //set as splash screen
    setStepperDefaults();
    pinMode(Z_STEP, OUTPUT);
    pinMode(Z_DIR, OUTPUT);
    pinMode(Z_EN, OUTPUT);
    digitalWrite(Z_EN, LOW);

    pinMode(X_STEP, OUTPUT);
    pinMode(X_DIR, OUTPUT);
    pinMode(X_EN, OUTPUT);
    digitalWrite(X_EN, LOW);
    drawScreen();
    delay(1000); //delay for splashscreen
    homeStepper(Vstepper,X_MIN);
    delay(1000); //delay for splashscreen
    screen = 1; //set screen as main menu
    drawScreen(); //draw new screen
} //end setup

void loop() {
//get encoder & button value
  readEncoder();
  Vstepper.run(); //runs steppers even after leaving menu
  Hstepper.run();

  //test output
  if(okPressed){
    Serial.println("pressed");
  }
  if(okReleased){
    Serial.println("released");
  }
  if (ccw==1){
    Serial.println("Counterclockwise");
  }
  if (cw==1){
    Serial.println("Clockwise");
  }

  //Vstepper.run();
  //Hstepper.run();

  //menu system
    /*
     * screen 0: splash screen
     * screen 1: root menu
     *  -Manual Angle
     *    -vertical angle [angle]
     *    -horizontal angle [angle]
     *    -back
     *  -Pictures
     *    -number vertical (number)(angle)
     *    -number horizontal (number)(angle)
     *    -back
     *  -Start  
     *    -credits
     *    -start
     *      -settings display
     *      -curr pic / total pics
     *      -curr layer / total layers
     *      -maybe time?
     *    -back
     * 
     *  
     */
      update_menu();

        //what to do when selection is made (on release)
        if (okReleased == 1) { //okReleased memory is getting overwritten somewhere in the update_menu function or deeper (it calls drawScreen)
          Serial.print("selection switch ");
          Serial.println(okReleased);
            switch (screen){
              case 0: //splash screen
                //screen = 1; //goto root
                //let the splash screen play out because it's the calibration screen now
                break; //case 0 break
              case 1: //root screen
                switch (menu_current){
                  case 0: //manual angle
                    menu_current = 0; //reset cursor
                    screen = 2; //goto manual angle screen
                    break;
                  case 1: //pictures
                    menu_current = 0; //reset cursor
                    screen = 3; //goto pictures screen
                    break;
                  case 2: //camera
                    menu_current = 0; //reset cursor
                    screen = 4; //goto camera settings screen
                    break;
                  case 3: //start
                    menu_current = 0; //reset cursor
                    screen = 5; //goto start screen
                    break;
                }
                break; //case 1 break
              case 2: //manual angle screen
                switch (menu_current){
                  case 0: //vertical Angle
                    //***do vertical angle stuff
                    Serial.println("vertical angle");
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      Vstepper.run();
                      if (ccw==1) {
                        if (vAngleInt > 0) {
                            vAngleInt--;
                            drawScreen();
                            //call move steppers funtion
                            //Vstepper.runToNewPosition(angleToSteps(vAngleInt)); //blocking
                            Vstepper.moveTo(angleToSteps(vAngleInt));
                        }
                      }
                      if (cw==1) {       
                        if (vAngleInt < 180) {
                            vAngleInt++;
                            drawScreen();
                            //call move steppers function
                            //Vstepper.runToNewPosition(angleToSteps(vAngleInt));
                            Vstepper.moveTo(angleToSteps(vAngleInt));
                        }
                      }
                    } //while
                    Serial.println("left vertical angle");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 1: //Horizontal Angle
                    //***do horizontal angle stuff
                    Serial.println("horizontal angle");
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      Hstepper.run();
                      if (ccw==1) {
                        if (hAngleInt > -360) {
                            hAngleInt--;
                            drawScreen();
                            //call move steppers funtion
                            Hstepper.moveTo(angleToSteps(hAngleInt));
                        }
                      }
                      if (cw==1) {       
                        if (hAngleInt < 360) {
                            hAngleInt++;
                            drawScreen();
                            //call move steppers function
                            Hstepper.moveTo(angleToSteps(hAngleInt));
                        }
                      }
                    } //while
                    Serial.println("left horizontal angle");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 2: //Back
                    menu_current = 0; //reset cursor
                    screen = 1; //goto root screen
                    Serial.println("manual BACK");
                    break;
                }
                break; //case 2 break
              case 3: //pictures screen (auto angle settings)
                switch (menu_current){
                  case 0: //number vertical
                    Serial.println("vertical pictures");
                    //***do vertical picture stuff
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      if (ccw==1) {
                        if (vPicInt > 0) {
                            vPicInt--;
                            drawScreen();
                        }
                      }
                      if (cw==1) {       
                        if (vPicInt < 180) {
                            vPicInt++;
                            drawScreen();
                        }
                      }
                    } //while
                    Serial.println("left vertical picture");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 1: //number horizontal
                    Serial.println("horizontal pictures");
                    //***do horizontal picture stuff
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      if (ccw==1) {
                        if (hPicInt > 0) {
                            hPicInt--;
                            drawScreen();
                        }
                      }
                      if (cw==1) {       
                        if (hPicInt < 180) {
                            hPicInt++;
                            drawScreen();
                        }
                      }
                    } //while
                    Serial.println("left horizontal picture");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 2: //Back
                    menu_current = 1; //reset cursor
                    screen = 1; //goto root screen
                    Serial.println("pictures BACK");
                    break;
                }
                break; //case 3 break
              case 4: //camera settings screen
                Serial.println("camera settings");
                switch (menu_current){
                  case 0: //auto focus enable/disable
                    Serial.println("focus enable/disable");
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      if (ccw==1) {
                        autoFocus = LOW;
                        drawScreen();
                        }
                      else if (cw==1) {       
                        autoFocus = HIGH;
                        drawScreen();
                      }
                    } //while
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 1: //after picture wait time
                    Serial.println("after picture wait time");
                    okReleased = 0;
                    selected = 1;
                    Serial.println("selected set to 1");
                    drawScreen();
                    while(okReleased != 1){
                      readEncoder();
                      if (ccw==1) {
                        if (afterPicWaitInt > 0) { //time in ms
                            afterPicWaitInt-=100; //subtract 100ms per notch
                            drawScreen();
                        }
                      }
                      if (cw==1) {       
                        if (afterPicWaitInt < 20000) {
                            afterPicWaitInt+=100; //add 100ms per notch
                            drawScreen();
                        }
                      }
                    } //while
                    Serial.println("left after picture wait time");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 2: //Back
                    menu_current = 2; //reset cursor
                    screen = 1; //goto root screen
                    Serial.println("camera BACK");
                    break;
                }
                break; //case 4 break
              case 5: //start screen
                Serial.println("welcome to the start screen!");
                switch (menu_current){
                  case 0: //start
                    Serial.println("Starting!");
                    //Vpics does nothing
                    //Hpics is /360 # pics starting at 0

                    Hstepper.setCurrentPosition(0); //set current position as zero
                    turn = PicsToSteps(hPicInt);
                    //turn = angleToSteps(360/hPicInt); //rounding errors (100 pics = 3 deg per pic * 100 = 300 deg total, not 360)
                    for (int i=0; i<hPicInt;i++){
                      if (estop == HIGH){
                        Serial.println("estop high! break out of loop!");
                        break; //break out of the for loop
                      }
                      takePic();
                      autoMove(turn);
                      Hstepper.setCurrentPosition(0); //set current position as zero
                    }
                    if (estop == LOW){
                      autoMove( 360-(hPicInt*turn) ); //go back to 360 degrees when done
                      Hstepper.setCurrentPosition(0); //set current position as zero.
                    }
                    exitestop();
                    break;
                  case 1: //Back
                    menu_current = 3; //reset cursor
                    screen = 1; //goto root screen
                    Serial.println("start BACK");
                    break;
                }
                break; //case 5 break
            }
            drawScreen();
            Serial.println("selection switch end");
          }//end if button pressed
  //Hstepper.run();
} //end loop

void takePic(){
  //camera variables
//const int prePicTime = 1000;      //machine settle time before taking pic
//const int shutterLength = 500;     //time to hold the shutter button down
//const int focusLength = 1500;     //time to hold the focus button down
//
//char afterPicWaitStr[16];   //Char array to store post pic delay 
//int afterPicWaitInt = 0;    //number in ms for post pic delay
//bool autoFocus = HIGH;

  delay(prePicTime);
  if (autoFocus == HIGH){
    Serial.println("focusing");
    digitalWrite(focusPin,HIGH);    //press focus button
    delay(focusLength);           //stay pressed for focus length
    digitalWrite(focusPin,LOW);     // release focus button
    Serial.println("focused");
    delay(50);
  }
  Serial.println("taking picture");
  digitalWrite(shutterPin,HIGH);  //press shutter button
  delay(shutterLength);           //stay pressed for shutter length
  digitalWrite(shutterPin,LOW);   //release shutter button
  Serial.println("picture taken");
  delay(afterPicWaitInt);         //wait for exposure time to finish
}

void autoMove(int steps){
  //check for kill button every loop
  setStepperDefaults();
  Hstepper.moveTo(steps);
  while(Hstepper.distanceToGo() > 0){
    Hstepper.run();
    if(digitalRead(KILL_PIN) == LOW || estop == HIGH){ //low is pressed, high is released
      estop = HIGH;
      setStepperEmergency();
      Hstepper.setCurrentPosition(0); //set current position as zero
      Hstepper.moveTo(0);
      //Vstepper.setCurrentPosition(0); //set current position as zero
      //Vstepper.moveTo(0);
    }
  }
//  estop = LOW;
}

void homeStepper(AccelStepper myStepper, int EndStopPin){ //vertical calibration
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
void setStepperEmergency(){
    //Vstepper.setMaxSpeed(0.0); //set speed to 0
    //Vstepper.setAcceleration(500.0); //crank acc to max so it stops ASAP
    Hstepper.setMaxSpeed(0.0);
    Hstepper.setAcceleration(500.0);
}
void exitestop(){
    estop = LOW; //force estop to low after exiting loop.
    setStepperDefaults();
}
int PicsToSteps(int numPics){
  #define gearTeeth 540.0 //number of teeth on the device large gear (assuming gear was full in case of arch)
  #define stepperTeeth 13.0 //number of teeth on the stepper's gear
  #define stepper360 200.0 //number of steps for a full stepper rotation
  int steps = 0;
  // 540/13 = 41.5 full rotations of the stepper to get one full rotation of the ring gear
  // 200 steps per rotation of stepper
  // 41.5*200 = total stpes per full rotation of 360deg
  // 41.5*200/numpics = steps per pic
  steps = ( (gearTeeth/stepperTeeth)*stepper360/numPics );
  return steps;
  //4 pics = 90 deg per pic
  //41.5*200/4 = 
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

void readEncoder(){
  static int pos = 0; //local
  cw = 0; //global
  ccw = 0; //global
  static int valOk = 0; //local
  okPressed = 0; //global
  okReleased = 0; //global
  
  encoder.tick();
  newPos = encoder.getPosition();
  if (pos != newPos) {
    if (pos>newPos){
      cw=1;
      //Serial.println("cw");
    }
    else if (pos<newPos){
      ccw=1;
      //Serial.println("ccw");
    }
    //Serial.print(newPos);
    //Serial.println();
    pos = newPos;
  }
    newOk = !digitalRead(BTN_ENC);
  if (valOk != newOk){
    if (newOk ==1){
      okPressed = 1;
    }
    else if (newOk ==0){
      okReleased = 1;
    }
    valOk = newOk;
  }
}

void update_menu(){
  if (ccw==1) {
      if (menu_current > 0) {
          menu_current--;
          drawScreen();
      }
  }
  if (cw==1) {       
      if (menu_current < menu_max) {
          menu_current++;
          drawScreen();
      }
  }
}

void drawScreen() {
    u8g.firstPage();  
    do {
        whatToDraw();
    } while( u8g.nextPage() );
}

int calcPicAngle(int numPics, int max_angle){
  if(numPics == 0){
    return 0;
  }
  else{
    int tmp;
    tmp = (max_angle/(numPics));
    //Serial.println(tmp);
    return(tmp);
  }  
  //return 3;
}

void whatToDraw() {
    uint8_t i, h;
    int w, d;
    u8g.setFontRefHeightText();
    u8g.setFontPosTop(); //use top left as 0,0 instead of bottom left
    h = u8g.getFontAscent()-u8g.getFontDescent()+1; //height of font plus 1px buffer
    w = u8g.getWidth();
    
    switch (screen) {
        case 0: { //splash screen
            //menu_max=0;
            u8g.drawStr(32,16,"ScappCam");
            u8g.drawStr(32,32,"V1.4 (estop)");
              //V1.1 = fixed start menu drawing artifacts
              //V1.2 = fixed number of steps rounding issue
            u8g.drawBitmapP( 0, 16, 4, 32, scappcam_bitmap); //x,y,width/8,height
            //u8g.drawFrame(0,51,64,13);
            //press OK to calibrate
        } break;
        case 1: { //root screen
            menu_max=3;
            const char *main_menu[4] = { "Manual Angle", "Pictures", "Camera", "Start" };

            u8g.setDefaultForegroundColor();
            u8g.drawStr(8, 0, "Home Screen");
            u8g.drawHLine(0,h-1,128);

            //draw menu items
            for( i = 0; i < 4; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified 8px
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) { //draw menu selection highlight box
                    u8g.drawBox(0, (i+1)*h+1, w, h); //filled box cleaner location
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, (i+1)*h+1, main_menu[i]);
            }
         } break;

         case 2: { //Manual Angle sub menu
            menu_max=2;
            const char *angle_menu[3] = { "Vertical Angle", "Horizontal Angle", "Back" };
             for( i = 0; i < 3; i++ ) {
                Vstepper.run(); //fixes weird choppy stepper movement on manual mode
                Hstepper.run();
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, angle_menu[i]);
            }
            //draw variables
            u8g.setDefaultForegroundColor();
            u8g.drawStr(0, 52, "V:"); //draw V
            sprintf (vAngleStr, "%d%c", vAngleInt,0xB0); //build angle string (0xB0 is '°' degree symbol)
            u8g.drawStr( 18, 52, vAngleStr ); //draw angle value
            if (selected == 1 && menu_current == 0){//vertical selected
              u8g.drawFrame(0,51,64,13);
            }
            
            u8g.drawStr(64, 52, "H:");
            sprintf (hAngleStr, "%d%c", hAngleInt, 0xB0);
            u8g.drawStr( 82, 52, hAngleStr );
            if (selected == 1 && menu_current == 1){//horizontal selected
              u8g.drawFrame(64,51,64,13);
            }
         } break;

         case 3: {//pictures sub screen
            menu_max=2;
            const char *pics_menu[3] = { "Vertical Pics", "Horizontal Pics", "Back" };

             for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, pics_menu[i]);
                
                //draw variables
                u8g.setDefaultForegroundColor();
                
                u8g.drawStr(0, 52, "V:"); //draw V
                sprintf (vPicStr, "%d(%d%c)", vPicInt, calcPicAngle(vPicInt,90),0xB0); //build pic string
                u8g.drawStr( 18, 52, vPicStr ); //draw pic value
                if (selected == 1 && menu_current == 0){//vertical selected
                  u8g.drawFrame(0,51,64,13);
                }
                
                u8g.drawStr(64, 52, "H:");
                sprintf (hPicStr, "%d(%d%c)", hPicInt, calcPicAngle(hPicInt,360),0xB0 );
                u8g.drawStr( 82, 52, hPicStr );
                if (selected == 1 && menu_current == 1){//horizontal selected
                  u8g.drawFrame(64,51,64,13);
                }
            }
         } break;
         case 4: {//camera settings screen (afterPicWaitInt)
            menu_max=2;
            const char *camera_menu[3] = { "Autofocus", "Wait Time", "Back" };

             for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, camera_menu[i]);

                //draw variables
                u8g.setDefaultForegroundColor();

                //AF (y/n)
                if (autoFocus == HIGH){
                  u8g.drawStr( 8, 52, "enabled" ); //draw pic value
                }
                else{
                  u8g.drawStr( 0, 52, "disabled" ); //draw pic value
                }
                
                if (selected == 1 && menu_current == 0){//vertical selected
                  u8g.drawFrame(0,51,64,13);
                }

                //Wait time (ms)
                sprintf (afterPicWaitStr, "%d ms", afterPicWaitInt); //build pic string
                u8g.drawStr( 72, 52, afterPicWaitStr );
                if (selected == 1 && menu_current == 1){//horizontal selected
                  u8g.drawFrame(64,51,64,13);
                }
            }
         } break;

         case 5: {//start screen
            menu_max=1;
            const char *start_menu[2] = { "Start", "Back" };

             for( i = 0; i < 2; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, start_menu[i]);
            }
         } break;

         break;
    }
}
