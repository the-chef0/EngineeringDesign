#include <Servo.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
MCUFRIEND_kbv tft;
Servo servo1;

int screenWidth = 320;
int screenHeight = 240;
int cursorY = 132;
int16_t  x1, y1;
uint16_t w, h;
String prevSeconds;

void setup() {
  servo1.attach(11);
  rotate();
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  Serial.print("ID = 0x");
  Serial.println(identifier, HEX);
  if (identifier == 0xEFEF) identifier = 0x9486;
  
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(0x0);
  tft.setFont(&FreeSans18pt7b);
}

void loop() {
  String seconds = String((millis()/1000)%60);
  String minutes = String((millis()/60000)%60);
  String hours = String((millis()/3600000)%24);
  
  if (seconds.length() < 2) {
    seconds = "0" + seconds;
  }
  if (minutes.length() < 2) {
    minutes = "0" + minutes;
  }
  if (hours.length() < 2) {
    hours = "0" + hours;
  }
  
  if (seconds != prevSeconds) {
    rotate();
    tft.fillScreen(0x0);
    String timeString = hours + ":" + minutes + ":" + seconds;
    tft.getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(getCursorX(),getCursorY());
    tft.println(timeString);
    //tft.getTextBounds("Time since last dose:", 0, 0, &x1, &y1, &w, &h);
    //tft.setCursor(getCursorX(),getCursorY()-30);
    //tft.println("Time since last dose:");
    prevSeconds = seconds;
  }
}

void rotate() {
  servo1.write(0);
  delay(127);
  servo1.write(90);
}

int getCursorX() {
  return (screenWidth - w)/2;
}

int getCursorY() {
  return (screenHeight/2) + (h/2);
}
