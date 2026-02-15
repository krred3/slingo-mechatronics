#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

// RTC object
RTC_DS1307 rtc;

// use the simple constructor: (I2C addr, cols, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

char daysOfTheWeek[7][12] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void setup() {
  Serial.begin(9600);

  // LCD init
  lcd.begin();       // no parameters
  lcd.backlight();   // turn on backlight
  lcd.clear();

  // RTC init
  if (! rtc.begin()) {
    lcd.setCursor(0,0);
    lcd.print("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    lcd.setCursor(0,0);
    lcd.print("RTC not running");
    // you may want to rtc.adjust(...) here
  }
}

void loop() {
  DateTime now = rtc.now();

  // display date on line 0
  lcd.setCursor(0, 0);
  lcd.print("DATE ");
  lcd.print(now.day());
  lcd.print('/');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.year());

  // display time on line 1
  lcd.setCursor(0, 1);
  lcd.print("TIME ");
  print2d(now.hour());
  lcd.print(':');
  print2d(now.minute());
  lcd.print(':');
  print2d(now.second());

  delay(500);
}

// helper to leading‑zero two‑digit numbers
void print2d(int v) {
  if (v < 10) lcd.print('0');
  lcd.print(v);
}
