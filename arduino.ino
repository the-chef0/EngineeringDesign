#include <Servo.h>
#include <MCUFRIEND_kbv.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

//Pins for the NFC module
Adafruit_PN532 nfc((13), (12), (11), (10));

//Constructors for the screen and servo respectively
MCUFRIEND_kbv tft;
Servo servo1;

//These are used getCenterX and getCenterY for centering the text
int screenWidth = 320;
int screenHeight = 240;
int cursorY = 132;
int16_t  x1, y1;
uint16_t w, h;

//This is used to update the screen only when the displayed time changes
int prevSecond;

//Here we define the treatment schedule
//The code assumes that the arrays are of equal sizes and index i gives the time
//of the ith dose
int scheduleYears[] =  {21, 21};
int scheduleMonths[] = {10, 10};
int scheduleDays[] =   {10, 10};
int scheduleHours[] =  {15, 15};
int scheduleMinutes[] = {45, 50};
int counter = 0;
bool doseAvailable = false;

void setup() {
  //Initialize the clock
  setSyncProvider(RTC.get);
  
  //servo1.attach(9);
  rotate();
  Serial.begin(9600);
  
  //Initialize the display
  uint16_t identifier = tft.readID();
  if (identifier == 0xEFEF) identifier = 0x9486;
  tft.begin(identifier);
  tft.setRotation(3);

  //Initialize the NFC reader
  nfc.begin();
  nfc.SAMConfig();
}

void loop() {
  //In every iteration of the loop, we take the time from the clock
  int currYear = year();
  int currMonth = month();
  int currDay = day();
  int currHour = hour();
  int currMinute = minute();
  int currSecond = second();

  //If another second has passed since the last one, we update the screen
  //and check the time against the treatment plan
  if (currSecond != prevSecond) {

    int scheduleYear = scheduleYears[counter];
    int scheduleMonth = scheduleMonths[counter];
    int scheduleDay = scheduleDays[counter];
    int scheduleHour = scheduleHours[counter];
    int scheduleMinute = scheduleMinutes[counter];
    //Check if the next dose is available
    if (currYear >= scheduleYear &
        currMonth >= scheduleMonth &
        currDay >= scheduleDay &
        currHour >= scheduleHour &
        currMinute >= scheduleMinute) {

      //If so, we let the whole program know that a dose is available
      doseAvailable = true;
      counter = counter + 1;
      Serial.println("dose available");
    }

    //Then we calculate the time remaining until the next dose
    //to update the countdown
    int cntdwnHours = scheduleHour - currHour;
    int cntdwnMinutes = scheduleMinute - currMinute - 1;
    int cntdwnSeconds = 59 - currSecond;
    String cntdwnHoursStr = "";
    String cntdwnMinutesStr = "";
    String cntdwnSecondsStr = "";

    //Just formatting here - if we have a single digit we add a 0 in front
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

    //Update the screen with the current time
    updateTime(tft,timeString);
    //Keep track of what the previous second was so that we know when to update things
    prevSecond = currSecond;
  }

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  //In each iteration of the loop, we wait for 50ms to see if an NFC tag was read
  //The loop runs very quickly so the timeout doesn't matter - it seems to catch the NFC tag every time
  //I just had to put it there so that the loop wouldn't get stuck on this line waiting for a tag
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);

  //If a tag is scanned, we check if a dose is available
  if (success) {
    if (doseAvailable) {
      rotate();
      doseAvailable = false;
    }
    else {
    }
  }
}

//This function starts the servo and stops it 127 ms later
//Seems to be a good approximation of the 30 degrees that we need
void rotate() {
  servo1.write(0);
  delay(127);
  servo1.write(90);
}

//Auxillary functions for updating the display
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
