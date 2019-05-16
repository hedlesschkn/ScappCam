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

RotaryEncoder encoder(31, 33);
int newPos;
int cw,ccw;

int newOk;
int okPressed;
int okReleased;

//https://forum.arduino.cc/index.php?topic=151669.0
//const uint8_t rook_bitmap[] PROGMEM = {
//  0x00,0x00,       // 00000000 
//  0x55,0x55,       // 01010101
//  0x7f,0x7f,       // 01111111
//  0x3e,0x3e,       // 00111110
//  0x3e,0x3e,       // 00111110 
//  0x3e,0x3e,       // 00111110
//  0x3e,0x3e,       // 00111110 
//  0x7f,0x7f        // 01111111
//};

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
//get encoder value
  static int pos = 0;
  cw = 0;
  ccw = 0;
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

//get button value
  static int valOk = 0;
  okPressed = 0;
  okReleased = 0;
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


//  //test output
//  if(okPressed){
//    Serial.println("pressed");
//  }
//  if(okReleased){
//    Serial.println("released");
//  }
//  if (ccw==1){
//    Serial.println("Counterclockwise");
//  }
//  if (cw==1){
//    Serial.println("Clockwise");
//  }

  //menu system
    /*
     * screen 0: splash screen
     * screen 1: root menu
     *  -Manual Angle
     *    -height (angle)
     *    -horizontal
     *    -back
     *  -Pictures
     *    -number vertical (number)(angle)
     *    -number horizontal (number)(angle)
     *    -back
     *  -Start  
     *    settings disply
     *    go
     *      settings disply
     *      curr pic / total pics
     *      curr layer / total layers
     *    back
     * 
     *  
     */
      update_menu();

        if (okReleased) {      
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
                    break;
                  case 1: //Horizontal Angle
                    //***do horizontal angle stuff
                    Serial.println("horizontal angle");
                    break;
                  case 2: //Back
                    screen = 1; //goto root screen
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
                    break;
                  case 2: //Back
                    screen = 1; //goto root screen
                    break;
                }
                break; //case 4 break
            }
            drawScreen();   
          }//end if button pressed
} //end loop


void update_menu(){
  if (ccw==1) {
      if (menu_current > 0) {
          menu_current--;
          drawScreen();
      }
  }
  if (cw==1) {       
      if (menu_current < 2) {
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
    switch (screen) {
        case 0: { //splash screen
            u8g.drawStr(32,getFontLineSpacing(),"ScappCam");
            u8g.drawStr(32,getFontLineSpacing()+16,"Tim Hebert");
            u8g.drawBitmapP( 0, 16, 4, 32, scappcam_bitmap); //x,y,width/8,height
        } break;
        case 1: { //root screen
            const char *menu_strings[3] = { "Manual Angle", "Pictures", "Start" };
            uint8_t i, h;
            int w, d;

            u8g.setFontRefHeightText();
            u8g.setFontPosTop();

            h = u8g.getFontAscent()-u8g.getFontDescent();
            w = u8g.getWidth();
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

         case 2: { //Manual Angle sub menu
            const char *menu_strings[3] = { "Vertical Angle", "Horizontal Angle", "Back" };
            uint8_t i, h;
            int w, d;
            
            u8g.setFontRefHeightText();
            u8g.setFontPosTop();

            h = u8g.getFontAscent()-u8g.getFontDescent();
            w = u8g.getWidth();
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

         case 3: {//pictures sub screen
            const char *menu_strings[3] = { "Vertical Pics", "Horizontal Pics", "Back" };
            uint8_t i, h;
            int w, d;
            
            u8g.setFontRefHeightText();
            u8g.setFontPosTop();

            h = u8g.getFontAscent()-u8g.getFontDescent();
            w = u8g.getWidth();
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
            uint8_t i, h;
            int w, d;
            
            u8g.setFontRefHeightText();
            u8g.setFontPosTop();

            h = u8g.getFontAscent()-u8g.getFontDescent();
            w = u8g.getWidth();
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

int getFontLineSpacing() {
  return 24;
}
