//endstop defines
#define end_Xmin 3
#define end_Xmax 2
#define end_Ymin 14
#define end_Ymax 15
#define end_Zmin 18
#define end_Zmax 19

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Scapp Cam endstop test, RAMPS 1.4");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!digitalRead(end_Xmin) ){
    Serial.println("pressed"); //these 'MakerBot' endstops are normally open, need to use a !(not)
  }
  else{
    Serial.println("open");
  }
  delay(100);
}
