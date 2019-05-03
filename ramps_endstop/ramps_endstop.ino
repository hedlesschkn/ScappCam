//endstop defines
#define end_Xmin 3
#define end_Xmax 2
#define end_Ymin 14
#define end_Ymax 15
#define end_Zmin 18
#define end_Zmax 19

//WARNING! double check the wiring of the endstops.  
//I recieved an endstop with the servo-side red/black swapped and had to repin it

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Scapp Cam endstop test, RAMPS 1.4");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!digitalRead(end_Xmin) ){
    Serial.println("pressed Xmin"); //these 'MakerBot' endstops are normally open, need to use a !(not)
  }
  else if(!digitalRead(end_Xmax) ){
    Serial.println("pressed Xmax");
  }
  else{
    Serial.println("open");
  }
  delay(100);
}
