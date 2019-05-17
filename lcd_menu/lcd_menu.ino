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

#include <SPI.h>
#include <RotaryEncoder.h> //ManageLibraries > RotaryEncoder by Matthias Hertel
//#include "U8g2lib.h" //ManageLibraries > U8g2
#include <U8glib.h>
#include <Wire.h> 

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

//U8GLIB_SSD1351_128X128_332 u8g(8, 9, 7);
// Screen
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g(DOGLCD_SCK, DOGLCD_MOSI, DOGLCD_CS);
U8GLIB_ST7920_128X64_1X u8g(DOGLCD_SCK, DOGLCD_MOSI, DOGLCD_CS);

uint8_t screen = 0; //start on splash screen
uint8_t menu_current = 0;
uint8_t menu_max = 2;

RotaryEncoder encoder(31, 33);
int newPos;
int cw,ccw;

int newOk = 0;
int okPressed = 0;
int okReleased = 0;

int selected = 0;

char vAngleStr[16];   //Char array to store Vertical Angle as a string 
char hAngleStr[16];   //Char array to store Horizontal Angle as a string
int vAngleInt = 0;    //manually set vertical angle number
int hAngleInt = 0;    //manually set horizontal angle number

char vPicStr[16];   //Char array to store Vertical Pictures as a string 
char hPicStr[16];   //Char array to store Horizontal Pictures as a string
int vPicInt = 0;    //number of vertical pics
int hPicInt = 0;    //number of horizontal pics

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
#define TEMP_0_PIN          13   
#define TEMP_1_PIN          14
//use heated bed or hot end or fan as light controller?

void setup() {    
    Serial.begin(9600);
    Serial.println("ScappCam");
    u8g.begin(); // Added
    u8g.setFont(u8g_font_unifont); //10 pixel height
//  u8g.setDefaultForegroundColor();

    pinMode(BTN_ENC, INPUT);              // Set BTN_ENC as an unput, encoder button
    digitalWrite(BTN_ENC, HIGH);          // turn on pullup resistors
    drawScreen();
    delay(1000); //delay for splashscreen
    screen = 1;
    drawScreen();
} //end setup

void loop() {
//get encoder & button value
  readEncoder();

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
                screen = 1; //goto root
                break; //case 0 break
              case 1: //root screen
                switch (menu_current){
                  case 0: //manual angle
                    screen = 2; //goto manual angle screen
                    break;
                  case 1: //pictures
                    screen = 3; //goto pictures screen
                    break;
                  case 2: //start
                    screen = 4; //goto start screen
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
                      if (ccw==1) {
                        if (vAngleInt > 0) {
                            vAngleInt--;
                            drawScreen();
                            //call move steppers funtion
                        }
                      }
                      if (cw==1) {       
                        if (vAngleInt < 180) {
                            vAngleInt++;
                            drawScreen();
                            //call move steppers function
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
                      if (ccw==1) {
                        if (hAngleInt > -360) {
                            hAngleInt--;
                            drawScreen();
                            //call move steppers funtion
                        }
                      }
                      if (cw==1) {       
                        if (hAngleInt < 360) {
                            hAngleInt++;
                            drawScreen();
                            //call move steppers function
                        }
                      }
                    } //while
                    Serial.println("left horizontal angle");
                    selected = 0;
                    Serial.println("selected set to 0");
                    break;
                  case 2: //Back
                    screen = 1; //goto root screen
                    Serial.println("manual BACK");
                    break;
                }
                break; //case 2 break
              case 3: //pictures screen
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
                    screen = 1; //goto root screen
                    Serial.println("pictures BACK");
                    break;
                }
                break; //case 3 break
              case 4: //start screen
                Serial.println("welcome to the start screen!");
                switch (menu_current){
                  case 0: //credits
                    screen = 0;
                    break;
                  case 1: //start
                    //***do start stuff
                    Serial.println("Starting!");
                    //check for kill button every loop
                    break;
                  case 2: //Back
                    screen = 1; //goto root screen
                    break;
                }
                break; //case 4 break
            }
            drawScreen();
            Serial.println("selection switch end");
          }//end if button pressed
} //end loop

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
    tmp = (max_angle/(numPics +1));
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
            u8g.drawStr(32,16,"ScappCam");
            u8g.drawStr(32,32,"Tim Hebert");
            u8g.drawBitmapP( 0, 16, 4, 32, scappcam_bitmap); //x,y,width/8,height
            //u8g.drawFrame(0,51,64,13);
            //press OK to calibrate
        } break;
        case 1: { //root screen
            const char *menu_strings[3] = { "Manual Angle", "Pictures", "Start" };

            u8g.setDefaultForegroundColor();
            u8g.drawStr(8, 0, "Home Screen");
            u8g.drawHLine(0,h-1,128);

            //draw menu items
            for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified 8px
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) { //draw menu selection highlight box
                    u8g.drawBox(0, (i+1)*h+1, w, h); //filled box cleaner location
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, (i+1)*h+1, menu_strings[i]);
            }
         } break;

         case 2: { //Manual Angle sub menu
            const char *menu_strings[3] = { "Vertical Angle", "Horizontal Angle", "Back" };
             for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, menu_strings[i]);
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
            const char *menu_strings[3] = { "Vertical Pics", "Horizontal Pics", "Back" };

             for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, menu_strings[i]);
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
         case 4: {//start screen
            const char *menu_strings[3] = { "Credits", "Start", "Back" };

             for( i = 0; i < 3; i++ ) {
                //d = (w-u8g.getStrWidth(menu_strings[i]))/2; //center text
                d = 8; //left justified
                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h); //filled box
                    //u8g.drawFrame(0, i*h+1, w, h); //outline box (if disabling foreground/background color)
                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, menu_strings[i]);
            }
         } break;

         break;
    }
}
