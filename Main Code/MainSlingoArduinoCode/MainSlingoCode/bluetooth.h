#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class BluetoothModule {
public:
  static const uint8_t MENU_INDEX = 5;

  BluetoothModule(HardwareSerial &serialPort, LiquidCrystal_I2C &display)
    : btSerial(serialPort), lcd(display),
      btActive(false), playing(false),
      lastAction(0), revertPending(false) {}

  void begin(uint32_t baud=9600) {
    btSerial.begin(baud);
  }

  void enter() {
    btActive      = true;
    playing       = true;
    revertPending = false;
    lastAction    = millis();
    drawHeader();
    drawStatus("Track Playing");
    drawFooter();
  }

  void handle(bool ok, bool left, bool right, bool back, uint8_t &screen) {
    if (back) {
      btActive = false;
      screen   = 0;  // MAIN_MENU
      drawHeader();
      return;
    }
    if (ok) {
      btSerial.write('P');
      playing = !playing;
      drawStatus(playing ? "Track Playing" : "Track Paused");
      lastAction    = millis();
      revertPending = false;
      return;
    }
    if (left) {
      btSerial.write('B');
      drawStatus("Prev track");
      lastAction    = millis();
      revertPending = true;
      return;
    }
    if (right) {
      btSerial.write('N');
      drawStatus("Next track");
      lastAction    = millis();
      revertPending = true;
      return;
    }
  }

  void update() {
    if (revertPending && millis() - lastAction >= ACTION_MS) {
      drawStatus(playing ? "Track Playing" : "Track Paused");
      revertPending = false;
    }
  }

  bool active() const { return btActive; }

private:
  HardwareSerial &btSerial;
  LiquidCrystal_I2C &lcd;
  bool btActive, playing;
  unsigned long lastAction;
  bool revertPending;
  static const unsigned long ACTION_MS = 2000;

  void drawHeader() {
    lcd.setCursor(7,0); lcd.print("Slingo");
    if (btActive) { lcd.setCursor(15,0); lcd.write(byte(3)); }
    lcd.setCursor(16,0); lcd.write(byte(0)); lcd.print("100");
    clearRow(1);
  }

  void drawFooter() {
    lcd.setCursor(1,3);
    lcd.write(byte(1)); lcd.print("   OK   BACK   "); lcd.write(byte(2));
  }

  void drawStatus(const char *msg) {
    clearRow(2);
    String s(msg);
    int p = (20 - s.length())/2; if (p<0) p=0;
    lcd.setCursor(p,2);
    lcd.print(s);
  }

  void clearRow(uint8_t r){
    lcd.setCursor(0,r);
    for (uint8_t i=0;i<20;i++) lcd.print(' ');
  }
};

#endif // BLUETOOTH_H
