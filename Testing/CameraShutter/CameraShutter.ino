/*
 * Remotely activating focus and shutter of Cannon camera wiht 2.5mm TRS
 * Canon triggering:
 * tip - shutter
 * ring - focus
 * sleeve - ground
 * 
 * 
 * Optocoupler for isolation:
 * https://www.amazon.com/gp/product/B07GMHLL2M/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1
 * 
 * TRS cable:
 * ***amazon link***
 * RED    - tip     - shutter
 * WHITE  - ring    - focus
 * GREEN  - sleeve  - ground
 * 
 * 
 * white to V1
 * white to G1
 * red to V2
 * jumpers enabled (will this cause a ground float?)
 * 
 * focus servo 1, opto 1
 * shutter servo 2, opto 2
 */

//servo defines
#define servo1 11
#define servo2 6
#define servo3 5
#define servo4 4

const int focusPin = servo1;
const int shutterPin = servo2;

int preShutterTime = 1000;  //wait time before triggering shutter
int postShutterTime = 2000; //wait time after triggering shutter
int shutterLength = 500;     //time to hold the shutter button down
int focusLength = 1500;     //time to hold the focus button down

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Camera focus/shutter tester");
    pinMode(focusPin,OUTPUT);
    pinMode(shutterPin,OUTPUT);
}

void loop() {
  static char c;
  while (Serial.available()) {
    c = Serial.read();  //gets one byte from serial buffer
  }

  if (c == 'f'){
    Serial.println("focusing...");
    cameraFocus();
    Serial.println("focus complete");
  }
  else if (c == 's'){
    Serial.println("shutter pressed");
    cameraShutter();
    Serial.println("picture taken");
  }
  c = "";
}

void cameraFocus(){
  digitalWrite(focusPin,HIGH);    //press focus button
  delay(focusLength);           //stay pressed for focus length
  digitalWrite(focusPin,LOW);     // release focus button
  cameraShutter();                //call shutter function
}

void cameraShutter(){
  delay(preShutterTime);          //wait for steady shot
  digitalWrite(shutterPin,HIGH);  //press shutter button
  delay(shutterLength);           //stay pressed for shutter length
  digitalWrite(shutterPin,LOW);   //release shutter button
  delay(postShutterTime);         //wait for exposure time to finish
}
