#include <Servo.h>
#include <MCUFRIEND_kbv.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

Adafruit_PN532 nfc((13), (12), (11), (10));

MCUFRIEND_kbv tft;
Servo servo1;

int screenWidth = 320;
int screenHeight = 240;
int cursorY = 132;
int16_t  x1, y1;
uint16_t w, h;
int prevSecond;

int scheduleYears[] =  {21, 21};
int scheduleMonths[] = {10, 10};
int scheduleDays[] =   {10, 10};
int scheduleHours[] =  {15, 15};
int scheduleMinutes[] = {45, 50};
int counter = 0;
bool doseAvailable = false;

void setup() {
  setSyncProvider(RTC.get);
  //servo1.attach(9);
  rotate();
  Serial.begin(9600);
  uint16_t identifier = tft.readID();
  if (identifier == 0xEFEF) identifier = 0x9486;

  tft.begin(identifier);
  tft.setRotation(3);

  nfc.begin();
  nfc.SAMConfig();
}

void loop() {
  int currYear = year();
  int currMonth = month();
  int currDay = day();
  int currHour = hour();
  int currMinute = minute();
  int currSecond = second();

  if (currSecond != prevSecond) {

    int scheduleYear = scheduleYears[counter];
    int scheduleMonth = scheduleMonths[counter];
    int scheduleDay = scheduleDays[counter];
    int scheduleHour = scheduleHours[counter];
    int scheduleMinute = scheduleMinutes[counter];

    if (currYear >= scheduleYear &
        currMonth >= scheduleMonth &
        currDay >= scheduleDay &
        currHour >= scheduleHour &
        currMinute >= scheduleMinute) {

      doseAvailable = true;
      counter = counter + 1;
      Serial.println("dose available");
    }

    int cntdwnHours = scheduleHour - currHour;
    int cntdwnMinutes = scheduleMinute - currMinute - 1;
    int cntdwnSeconds = 59 - currSecond;
    String cntdwnHoursStr = "";
    String cntdwnMinutesStr = "";
    String cntdwnSecondsStr = "";

    if (cntdwnHours < 10) {
      cntdwnHoursStr = "0" + String(cntdwnHours);
    } else {
      cntdwnHoursStr = String(cntdwnHours);
    }

    if (cntdwnMinutes < 10) {
      cntdwnMinutesStr = "0" + String(cntdwnMinutes);
    } else {
      cntdwnMinutesStr = String(cntdwnMinutes);
    }

    if (cntdwnSeconds < 10) {
      cntdwnSecondsStr = "0" + String(cntdwnSeconds);
    } else {
      cntdwnSecondsStr = String(cntdwnSeconds);
    }

    String timeString = cntdwnHoursStr + ":" + cntdwnMinutesStr + ":" + cntdwnSecondsStr;
    updateTime(tft,timeString);
    prevSecond = currSecond;
  }

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);j

  if (success) {
    if (doseAvailable) {
      rotate();
      doseAvailable = false;
    }
    else {
    }
  }
}

void rotate() {
  servo1.write(0);
  delay(127);
  servo1.write(90);
}

void updateTime(MCUFRIEND_kbv screen, String string) {
  screen.fillScreen(0x0);
  screen.getTextBounds(string, 0, 0, &x1, &y1, &w, &h);
  //tft.setFont(&FreeSans12pt7b);
  //screen.setCursor(getCenterX(),getCenterY()-30);
  //screen.println("Next dose in:");
  
  tft.setFont(&FreeSans18pt7b);
  screen.setCursor(getCenterX(), getCenterY());
  screen.println(string);
}

int getCenterX() {
  return (screenWidth - w) / 2;
}

int getCenterY() {
  return (screenHeight / 2) + (h / 2);
}
