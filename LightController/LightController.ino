#include <FastLED.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 5); // RX, TX
#define REDPIN   10
#define GREENPIN 11
#define BLUEPIN  9
#define BUTTONPIN 12

const byte numChars = 10;
char SerialHSV[numChars];   // an array to store the received data

boolean newData = false;

int hue = 255;
int sat = 255;
int val = 255;
boolean bluetooth = true;
int buttonState = 0;
int prevButtonState = 0;

void showAnalogRGB( const CRGB& rgb)
{
  analogWrite(REDPIN,   rgb.r );
  analogWrite(GREENPIN, rgb.g );
  analogWrite(BLUEPIN,  rgb.b );
}

void colorBars()
{
  showAnalogRGB( CRGB::Red );   delay(250);
  showAnalogRGB( CRGB::Green ); delay(250);
  showAnalogRGB( CRGB::Blue );  delay(250);
  showAnalogRGB( CRGB::Black ); delay(250);
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

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

void showColor(){
    if (newData == true) {
      String StringHSV = String(SerialHSV);
      
      if(StringHSV.length() == 9){
        hue = StringHSV.substring(0,3).toInt();
        sat = StringHSV.substring(3,6).toInt();
        val = StringHSV.substring(6,10).toInt();
        Serial.println("Red:" + String(hue) + " Green:" + String(sat) + " Blue:" + String(val));
        showAnalogRGB( CRGB( hue, sat, val) );
//        Serial.println("Flash: Hue:" + String(hue) + " Saturation:" + String(sat) + " Value:" + String(val));
//        showAnalogRGB( CHSV( hue, sat, val) );
      };
      newData = false;
    }
}
void blueFade(){
  for(int x = 0; x < 255; x++){
    showAnalogRGB( CHSV(128,255,x) );
    delay(2);
  }
  for(int x = 254; x < 255 && x > 1; x--){
    showAnalogRGB( CHSV(128,255,x) );
    delay(2);
  }
}
void setup() {
  pinMode(REDPIN,   OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN,  OUTPUT);
  pinMode(BUTTONPIN,  INPUT);
  Serial.begin(9600);
  mySerial.println("Connected via bluetooth to Howard's Light thingy");
  Serial.println("Connected via USB to Howard's Light thingy");
  mySerial.begin(9600);
  colorBars();
  blueFade();
  showAnalogRGB( CHSV(0,0,0) );
}

void loop() {
  
  if(bluetooth == true){
    recvWithEndMarker();
    showColor();
  }else{
    showAnalogRGB( CRGB(255,80,10) );
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
      showAnalogRGB( CHSV(0,0,0) );
    }
  }
  prevButtonState = buttonState;
  
}
