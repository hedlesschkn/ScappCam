#include <SPI.h>
#include <RotaryEncoder.h> //Matthias Hertel
//#include "U8g2lib.h" //ManageLibraries > U8g2
#include <U8glib.h>
#include <Wire.h>

#define PULLUP true     
#define INVERT true    
#define DEBOUNCE_MS 20   

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

char vAngleStr[16];                         //Char array to store Vertical Angle as a string 
char hAngleStr[16];                         //Char array to store Horizontal Angle as a string
int vAngleInt = 0;
int hAngleInt = 0;

char vPicsStr[16];                         //Char array to store Vertical Pictures as a string 
char hPicsStr[16];                         //Char array to store Horizontal Pictures as a string
int vPicsInt = 0;
int hPicsInt = 0;

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

void setup() {    
    Serial.begin(9600);
    Serial.println("ScappCam");
    u8g.begin(); // Added
    u8g.setFont(u8g_font_unifont);       
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
                    //***do vertical angle stuff
                    Serial.println("vertical pictures");
                    break;
                  case 1: //number horizontal
                    //***do horizontal angle stuff
                    Serial.println("horizontal pictures");
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
            u8g.drawFrame(0,51,64,13);
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
                    u8g.drawBox(0, (i+1)*h+1, w, h); //filled box
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
            sprintf (vAngleStr, "%d(%d)", vAngleInt, 3); //build angle string
            u8g.drawStr( 18, 52, vAngleStr ); //draw angle value
            if (selected == 1 && menu_current == 0){//vertical selected
              u8g.drawFrame(0,51,64,13);
            }
            
            u8g.drawStr(64, 52, "H:");
            sprintf (hAngleStr, "%d(%d)", hAngleInt, 10);
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
