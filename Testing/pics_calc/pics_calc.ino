String readString;

int temparray[360];
int myarray[360];

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Seral String Input");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();  //gets one byte from serial buffer
    readString += c; //makes the string readString
    delay(2);  //slow looping to allow buffer to fill with next character
  }

  if (readString.length() >0) {
    Serial.print("input: ");
    Serial.println(readString);  //so you can see the captured string 
    int n = readString.toInt();  //convert readString into a number

    readString=""; //empty for next input
    Serial.print("output: ");
    //Serial.println(calcPicAngle(n,360));
    getpics360(n);
    for(int i = 0; i < 360; i++)
      {
        Serial.print(temparray[i]);
        Serial.print(", ");
      }
  } 

  
}

int getpics360(int numPics){
  for (int i=0; i<numPics; i++){
    temparray[i] = (360/numPics)*i;
  }
  return temparray;
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
