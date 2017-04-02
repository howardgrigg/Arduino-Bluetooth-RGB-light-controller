#include <FastLED.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 5); // RX, TX
#define REDPIN   9
#define GREENPIN 10
#define BLUEPIN  11
#define BUTTONPIN 12

const byte numChars = 14;
char SerialHSV[numChars];   // an array to store the received data

boolean newData = false;
boolean bluetooth = true;
boolean disco = false;
boolean alarm = false;
boolean strobe = false;

int currentHue, currentSat, currentVal;
int oldHue, oldSat, oldVal;
int targetHue, targetSat, targetVal;
int totalSteps;
int currentStep;

int buttonState = 0;
int prevButtonState = 0;

void setup() {
  pinMode(REDPIN,   OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN,  OUTPUT);
  pinMode(BUTTONPIN,  INPUT);
  Serial.begin(9600);
  mySerial.println("Connected via bluetooth to Howard's Light thingy");
  Serial.println("Connected via USB to Howard's Light thingy");
  mySerial.begin(9600);
//  colorBars();
  blueFade();
  showAnalogRGB( CHSV(0,0,0) );
}

void loop() {
  
  if(bluetooth == true){
    recvWithEndMarker();
    updateData();
    if(disco == false && alarm == false && strobe == false){
      showColor();
    }else if(disco == true){
      int discoColour = random(0, 255);
      int discoDelay =random(100, 500);
      showAnalogRGB(CHSV( discoColour, 255, 255));
      delay(discoDelay);
    }else if(alarm == true){
      showAnalogRGB(CHSV( 0, 255, 255));
      delay(250);
      showAnalogRGB(CHSV( 160, 255, 255));
      delay(250);
    }else if(strobe == true){
      Serial.println("Flash");
      showAnalogRGB(CHSV( 0, 0, 255));
      delay(50);
      showAnalogRGB(CHSV( 0, 0, 0));
      delay(200);
    }
  }else{
    //showAnalogRGB( CRGB(255,80,10) ); // too pink
    //showAnalogRGB( CRGB(255,197,55) ); // little bit too white
    showAnalogRGB( CRGB(255,160,30) ); // Reasonable tungston match
  }
  
  buttonState = digitalRead(BUTTONPIN);
  
  if(buttonState != prevButtonState && buttonState == 1){
    if(bluetooth == true){
      bluetooth = false;
      Serial.println("Bluetooth off");
    }else{
      bluetooth = true;
      Serial.println("Bluetooth on");
      blueFade();
      targetHue = currentHue;
      oldHue = targetHue;
      targetSat = currentSat;
      targetVal = currentVal;
      currentSat = 0;
      currentVal = 0;
      totalSteps = 500;
      currentStep = 0;
      showColor();
    }
  }
  prevButtonState = buttonState;
  delay(1);
}
void showAnalogRGB( const CRGB& rgb)
{
  analogWrite(REDPIN,   rgb.r );
  analogWrite(GREENPIN, rgb.g );
  analogWrite(BLUEPIN,  rgb.b );
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (mySerial.available() > 0 && newData == false) {
        rc = mySerial.read();

        if (rc != endMarker) {
            SerialHSV[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            SerialHSV[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void updateData(){
  if (newData == true) {
    String StringHSV = String(SerialHSV);
      
    if(StringHSV.length() == 13){
      oldHue = currentHue;
      oldSat = currentSat;
      oldVal = currentVal;
      
      targetHue = StringHSV.substring(0,3).toInt();
      targetSat = StringHSV.substring(3,6).toInt();
      targetVal = StringHSV.substring(6,9).toInt();
      totalSteps = StringHSV.substring(9,14).toInt();
      Serial.println("Inputs: Hue:" + String(targetHue) + " Saturation:" + String(targetSat) + " Value:" + String(targetVal) + " Steps:" + String(totalSteps));
      
      if (totalSteps == 0){totalSteps++;}; // if sent a string of 0000 then convert to 1 step so it'll do something
      
      if(targetVal == 0){ // if fading to black don't mess with the colours or saturation
        targetHue = currentHue;
        targetSat = currentSat;
      };
      
      int difference = oldHue - targetHue; // If fading colour with a difference of more than 128 then go the correct direction
      if(difference > 128 || difference < -128){
        if(difference > 0){
          targetHue = targetHue + 255;
        };
        if(difference < 0){
          currentHue = currentHue + 255;
          oldHue = currentHue;
        };
        Serial.println("Big Gap");
      };
      
      if(oldVal == 0){ // this line stops it from scrolling through all the colours if the lights are currently off as it fades on
        oldHue = targetHue;
      };
      
      currentStep = 0;
      
      Serial.println("New targets: Hue:" + String(targetHue) + " Saturation:" + String(targetSat) + " Value:" + String(targetVal) + " Steps:" + String(totalSteps));
    };
    if(StringHSV == "disco"){
      Serial.println("Disco time!");
      disco = true;
    }else if(disco == true){
      Serial.println("Disco over");
      disco = false;
    }

    if(StringHSV == "alarm"){
      Serial.println("Oh dear!");
      alarm = true;
    }else if(alarm == true){
      Serial.println("Problem solved");
      alarm = false;
    }

    if(StringHSV == "strobe"){
      Serial.println("Party like it's 1999!");
      strobe = true;
    }else if(strobe == true){
      Serial.println("Party over");
      strobe = false;
    }
    
    newData = false;
  }
}

void showColor(){
  if (currentStep != totalSteps + 1){
//    Serial.println("Fadeinput: "+String(oldHue)+", "+String(currentHue)+", "+String(targetHue)+", "+String(currentStep)+", "+String(totalSteps));
    currentHue = fadeStep(oldHue, currentHue, targetHue, currentStep, totalSteps);
    currentSat = fadeStep(oldSat, currentSat, targetSat, currentStep, totalSteps);
    currentVal = fadeStep(oldVal, currentVal, targetVal, currentStep, totalSteps);
    currentStep++;
//    Serial.println(String(currentHue)+", "+String(currentSat)+", "+String(currentVal)+", Step:"+String(currentStep));
    showAnalogRGB( CHSV( currentHue, currentSat, currentVal) );
  }
  if ((currentStep == totalSteps + 1) && targetHue > 255){
    targetHue = targetHue - 255;
    currentHue = targetHue;
    Serial.println("corrected value to "+String(currentHue));
  }
}

int fadeStep(float oldLevel, int currentLevel, float targetLevel, int currentStep, int totalSteps){
//  Serial.println(String(oldLevel)+", "+String(currentLevel)+", "+String(targetLevel)+", "+String(currentStep)+", "+String(totalSteps));
  float change = (targetLevel-oldLevel) / totalSteps;
  float newLevel = oldLevel + (change * currentStep);
  int newLevelInt = round(newLevel);
  return newLevelInt;
}

void blueFade(){
  for(int x = 0; x < 100; x++){
    showAnalogRGB( CHSV(128,255,x) );
    delay(4);
  }
  for(int x = 99; x < 100 && x > 1; x--){
    showAnalogRGB( CHSV(128,255,x) );
    delay(4);
  }
}


