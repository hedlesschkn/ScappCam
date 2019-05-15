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

uint8_t screen = 1;
uint8_t menu_current = 0;

RotaryEncoder encoder(31, 33);
int newPos;
int cw,ccw;

int newOk;
int okPressed;
int okReleased;


void setup() {    
    Serial.begin(9600);
    Serial.println("ScappCam");
    u8g.begin(); // Added
    u8g.setFont(u8g_font_unifont);       
//  u8g.setDefaultForegroundColor();

    //drawScreen();
    pinMode(BTN_ENC, INPUT);              // Set BTN_ENC as an unput, encoder button
    digitalWrite(BTN_ENC, HIGH);          // turn on pullup resistors
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
     *    -height (angle)
     *    -horizontal
     *  -Pictures 
     *  
     */
//    if (screen == 1) {
//        if (ccw==1) {
//          Serial.println("ccw");
//            if (menu_current > 0) {
//                menu_current--;
//                drawScreen();
//            }
//        }
//
//        if (cw==1) {       
//            if (menu_current < 2) {
//                menu_current++;
//                drawScreen();
//            }
//        }
//
//        if (!digitalRead(BTN_ENC)) {
//          Serial.println("ok pressed");       
//            switch (menu_current) {          
//                case 0: 
//                    screen = 2;
//                    break;
//                case 1:
//                    screen = 3;
//                    break;
//                case 2:
//                    screen = 0;
//                    break;
//                break;
//            }
//            drawScreen();
//        }
//    }
//
//    if (screen == 2 || screen == 3) {
//        if (menu.wasReleased()) {
//            screen = 1;
//            menu_current = 0;
//            drawScreen();
//        }
//    }
} //end loop




void drawScreen() {
    u8g.firstPage();  
    do {
        whatToDraw();
    } while( u8g.nextPage() );
}

void whatToDraw() {
    switch (screen) {
        case 0: {                           
            u8g.drawStr(0,getFontLineSpacing(),"Init");
        } break;
        case 1: {   
            const char *menu_strings[3] = { "option1", "option2", "Back" };
            uint8_t i, h;
            int w, d;

            u8g.setFontRefHeightText();
            u8g.setFontPosTop();

            h = u8g.getFontAscent()-u8g.getFontDescent();
            w = u8g.getWidth();
            for( i = 0; i < 3; i++ ) {
                d = (w-u8g.getStrWidth(menu_strings[i]))/2;
//                u8g.setDefaultForegroundColor();
                if ( i == menu_current ) {
                    u8g.drawBox(0, i*h+1, w, h);
//                    u8g.setDefaultBackgroundColor();
                }
                u8g.drawStr(d, i*h, menu_strings[i]);
            }
         } break;

         case 2: {                         
             u8g.drawStr(0,getFontLineSpacing(),"Option1");
         } break;

         case 3: {                        
             u8g.drawStr(0,getFontLineSpacing(),"Option2");
         } break;

         break;
    }
}

int getFontLineSpacing() {
  return 24;
}
