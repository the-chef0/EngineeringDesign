#include <Servo.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <DS3231.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define PN532_SCK  (13)
#define PN532_MISO (12)
#define PN532_MOSI (11)
#define PN532_SS   (10)
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


MCUFRIEND_kbv tft;
Servo servo1;
DS3231 clock;
bool century = false;
bool h12Flag;
bool pmFlag;

int screenWidth = 320;
int screenHeight = 240;
int cursorY = 132;
int16_t  x1, y1;
uint16_t w, h;
int prevSecond;

int scheduleYears[] =  {21,21,21,21,21};
int scheduleMonths[] = {10,10,10,10,10};
int scheduleDays[] =   {4,4,4,4,4};
int scheduleHours[] =  {11,11,11,12,12};
int scheduleMinutes[] = {48,50,55,0,5};
int counter = 0;
bool doseAvailable = false;

void setup() {
  servo1.attach(9);
  rotate();
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  if (identifier == 0xEFEF) identifier = 0x9486;
  
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(0x0);
  tft.setFont(&FreeSans18pt7b);

  //clock.setMinute(34);
  //clock.setHour(11);
  //clock.setDate(4);
  //clock.setMonth(10);
  //clock.setYear(21);

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop() {
  int currYear = clock.getYear();
  int currMonth = clock.getMonth(century);
  int currDay = clock.getDate();
  int currHour = clock.getHour(h12Flag, pmFlag);
  int currMinute = clock.getMinute();
  int currSecond = clock.getSecond();
  
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
    Serial.println(timeString);
    prevSecond = currSecond;
  }

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    if (doseAvailable) {
      Serial.println("Dispensed.");
      delay(500);
      rotate();
      doseAvailable = false;
    }
    else {
      Serial.println("Next dose available in:");
      delay(500);
    }
  }
}

void rotate() {
  servo1.write(0);
  delay(127);
  servo1.write(90);
}

void updateDisplay(MCUFRIEND_kbv screen, String string) {
  screen.fillScreen(0x0);
  screen.getTextBounds(string, 0, 0, &x1, &y1, &w, &h);
  screen.setCursor(getCursorX(),getCursorY());
  screen.println(string);
}

int getCursorX() {
  return (screenWidth - w)/2;
}

int getCursorY() {
  return (screenHeight/2) + (h/2);
}
