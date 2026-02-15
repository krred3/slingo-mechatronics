#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include "DFRobot_DF2301Q.h"

#include "MotionSensor.h"
#include "LEDDimmer.h"
#include "FMRadio.h"
#include "bluetooth.h"

// Pin map
const uint8_t BTN_LEFT  = 7;
const uint8_t BTN_RIGHT = 4;
const uint8_t BTN_OK    = 6;
const uint8_t BTN_BACK  = 5;
const uint8_t PIR_PIN   = 3;
const uint8_t PIR_LED   = 2;
const uint8_t DIM_LED   = 2;

// LCD & custom chars
LiquidCrystal_I2C lcd(0x27,20,4);
byte BATTERY_FULL[8] = {B01110,B11111,B11111,B11111,B11111,B11111,B11111,B11111};
byte ARROW_L[8]      = {B00100,B01100,B11111,B01100,B00100,0,0,0};
byte ARROW_R[8]      = {B00100,B00110,B11111,B00110,B00100,0,0,0};
byte BT_ICON[8]      = {B00100,B01110,B00100,B01110,B00100,B01110,B00100,0};

// Voice module
DFRobot_DF2301Q_I2C DF2301Q;

// Other modules
MotionSensorModule motionSensor(PIR_PIN, PIR_LED);
LEDDimmerModule    ledDimmer(DIM_LED);
FMRadioModule      fmRadio;
BluetoothModule    bluetooth(Serial, lcd);

// UI state
enum Screen {
  MAIN_MENU,
  MOTION_SENSOR,
  LED_MODULE,
  FM_MODULE,
  SPEAKER_MODULE,  // stub
  HF_MODULE,
  BT_MODULE
};
Screen screen = MAIN_MENU;

const char* ITEMS[] = {
  "Motion Sensor","LED","FM Radio","Speaker","Hands Free","Bluetooth"
};
const uint8_t ITEM_CNT = sizeof(ITEMS)/sizeof(ITEMS[0]);
uint8_t currentIdx = 0;

// Sub‑menu focus/state
uint8_t msFocus = 1, hfFocus = 1;
bool    hfEnabled = false;

// Helpers
void clearRow(uint8_t r){
  lcd.setCursor(0,r);
  for(int i=0;i<20;i++) lcd.print(' ');
}
bool pressed(uint8_t p){
  static bool prev[10]={0};
  bool now = !digitalRead(p), hit = now && !prev[p];
  prev[p] = now; return hit;
}

// Clock
void initClock(){
  int hh = 10*(__TIME__[0]-'0') + (__TIME__[1]-'0');
  int mm = 10*(__TIME__[3]-'0') + (__TIME__[4]-'0');
  setTime(hh,mm,0,1,1,24);
}
void drawClock(){
  char buf[6];
  sprintf(buf,"%02d:%02d",hour(),minute());
  lcd.setCursor(0,0);
  lcd.print(buf);
}

// Header/Footer
void drawHeader(){
  drawClock();
  lcd.setCursor(7,0); lcd.print("Slingo");
  if(bluetooth.active()) lcd.setCursor(15,0), lcd.write(byte(3));
  lcd.setCursor(16,0); lcd.write(byte(0)); lcd.print("100");
  clearRow(1);
}
void drawFooter(){
  lcd.setCursor(1,3);
  lcd.write(byte(1)); lcd.print("   OK   BACK   "); lcd.write(byte(2));
}

// Menu Carousel
void drawMenu(){
  String curr=ITEMS[currentIdx], loop;
  for(uint8_t i=0;i<ITEM_CNT;i++){
    loop += ITEMS[(currentIdx+i+ITEM_CNT-1)%ITEM_CNT];
    if(i<ITEM_CNT-1) loop += "   ";
  }
  int pad = (20-curr.length())/2; if(pad<0) pad=0;
  int pos = loop.indexOf(curr);
  int start = (pos-pad+loop.length())%loop.length();
  String win;
  for(uint8_t c=0;c<20;c++) win+=loop[(start+c)%loop.length()];
  lcd.setCursor(0,2); lcd.print(win);
}

// Motion Sensor UI
void drawMotionChooser(){
  clearRow(2);
  String l = msFocus==0?"[Enable]":"Enable";
  String r = msFocus==1?"[Disable]":"Disable";
  String line = l+"   "+r;
  int p=(20-line.length())/2; if(p<0)p=0;
  lcd.setCursor(p,2); lcd.print(line);
}
void flashMSStatus(bool e){
  clearRow(2);
  const char*m=e?"Enabled":"Disabled";
  int p=(20-strlen(m))/2; lcd.setCursor(p,2); lcd.print(m);
  delay(2000); clearRow(2);
}

// LED UI
void drawLedHeader(){
  clearRow(1);
  const char*h="Brightness";
  int p=(20-strlen(h))/2;
  lcd.setCursor(p,1); lcd.print(h);
}
void drawLedValue(){
  clearRow(2);
  char buf[12]; sprintf(buf,"-  %d  +",ledDimmer.get());
  String s(buf); int p=(20-s.length())/2;
  lcd.setCursor(p,2); lcd.print(s);
}

// FM UI
void drawFmHeader(){
  clearRow(1);
  const char*h="Radio Station";
  int p=(20-strlen(h))/2;
  lcd.setCursor(p,1); lcd.print(h);
}
void drawFmValue(){
  clearRow(2);
  char buf[12]; dtostrf(fmRadio.current(),4,1,buf);
  String s=String("-  ")+buf+"  +"; int p=(20-s.length())/2;
  lcd.setCursor(p,2); lcd.print(s);
}

// Hands‑Free UI
void drawHFChooser(){
  clearRow(2);
  String l=hfFocus==0?"[Enable]":"Enable";
  String r=hfFocus==1?"[Disable]":"Disable";
  String line=l+"   "+r;
  int p=(20-line.length())/2; if(p<0)p=0;
  lcd.setCursor(p,2); lcd.print(line);
}
void flashHFStatus(bool e){
  clearRow(2);
  const char*m=e?"HF Enabled":"HF Disabled";
  int p=(20-strlen(m))/2; lcd.setCursor(p,2); lcd.print(m);
  delay(2000); clearRow(2);
}

// SETUP
void setup(){
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BTN_RIGHT,INPUT_PULLUP);
  pinMode(BTN_OK,INPUT_PULLUP);
  pinMode(BTN_BACK,INPUT_PULLUP);

  // Voice module init
  Serial.begin(115200);
  while(!DF2301Q.begin()){
    Serial.println("Voice comm error"); delay(3000);
  }
  DF2301Q.setVolume(4);
  DF2301Q.setMuteMode(0);
  DF2301Q.setWakeTime(15);

  // Other modules
  motionSensor.begin();
  ledDimmer.begin();
  fmRadio.begin();
  bluetooth.begin();

  initClock();
  lcd.begin(); lcd.backlight();
  lcd.createChar(0,BATTERY_FULL);
  lcd.createChar(1,ARROW_L);
  lcd.createChar(2,ARROW_R);
  lcd.createChar(3,BT_ICON);

  drawHeader(); drawMenu(); drawFooter();
}

// LOOP
void loop(){
  static int prevM=-1;
  if(minute()!=prevM){
    prevM=minute(); drawClock();
  }

  // MAIN_MENU
  if(screen==MAIN_MENU){
    if(pressed(BTN_LEFT)){
      currentIdx = currentIdx?currentIdx-1:ITEM_CNT-1;
      drawMenu();
    }
    if(pressed(BTN_RIGHT)){
      currentIdx=(currentIdx+1)%ITEM_CNT;
      drawMenu();
    }
    if(pressed(BTN_OK)){
      switch(currentIdx){
        case 0:
          screen=MOTION_SENSOR;
          msFocus=motionSensor.isEnabled()?0:1;
          drawMotionChooser();
          break;
        case 1:
          screen=LED_MODULE;
          drawLedHeader(); drawLedValue();
          break;
        case 2:
          fmRadio.enable(true);
          screen=FM_MODULE;
          drawFmHeader(); drawFmValue();
          break;
        case 3:
          screen=SPEAKER_MODULE;  // stub
          drawHeader(); drawFooter();
          break;
        case 4:
          screen=HF_MODULE;
          drawHFChooser();
          break;
        case 5:
          bluetooth.enter();
          screen=BT_MODULE;
          break;
      }
    }
  }
  // MOTION_SENSOR
  else if(screen==MOTION_SENSOR){
    if(pressed(BTN_LEFT)||pressed(BTN_RIGHT)){
      msFocus^=1; drawMotionChooser();
    }
    else if(pressed(BTN_OK)){
      bool st=(msFocus==0);
      if(st!=motionSensor.isEnabled()){
        st?motionSensor.enable():motionSensor.disable();
        flashMSStatus(st);
      }
      msFocus=motionSensor.isEnabled()?0:1;
      drawMotionChooser();
    }
    else if(pressed(BTN_BACK)){
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
  }
  // LED_MODULE
  else if(screen==LED_MODULE){
    if(pressed(BTN_LEFT)){ ledDimmer.dec(); drawLedValue(); }
    else if(pressed(BTN_RIGHT)){ ledDimmer.inc(); drawLedValue(); }
    else if(pressed(BTN_BACK)){
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
  }
  // FM_MODULE
  else if(screen==FM_MODULE){
    if(pressed(BTN_LEFT)){ fmRadio.prev(); drawFmValue(); }
    else if(pressed(BTN_RIGHT)){ fmRadio.next(); drawFmValue(); }
    else if(pressed(BTN_BACK)){
      fmRadio.enable(false);
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
  }
  // SPEAKER_MODULE (stub)
  else if(screen==SPEAKER_MODULE){
    if(pressed(BTN_BACK)){
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
  }
  // HF_MODULE
  else if(screen==HF_MODULE){
    if(pressed(BTN_LEFT)||pressed(BTN_RIGHT)){
      hfFocus^=1; drawHFChooser();
    }
    else if(pressed(BTN_OK)){
      hfEnabled=(hfFocus==0);
      flashHFStatus(hfEnabled);
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
    else if(pressed(BTN_BACK)){
      screen=MAIN_MENU; drawHeader(); drawMenu(); drawFooter();
    }
  }
  // BT_MODULE
  else if(screen==BT_MODULE){
    bool o=pressed(BTN_OK), l=pressed(BTN_LEFT),
         r=pressed(BTN_RIGHT), b=pressed(BTN_BACK);
    uint8_t s=(uint8_t)screen;
    bluetooth.handle(o,l,r,b,s);
    screen=(Screen)s;
    if(screen!=BT_MODULE){
      drawMenu(); drawFooter();
    }
  }

  // Updates
  bluetooth.update();
  motionSensor.update();
  ledDimmer.update();

  // Hands‑Free commands via I²C
  if(hfEnabled){
    uint8_t cmd = DF2301Q.getCMDID();
    if(cmd){
      uint8_t s = (uint8_t)BT_MODULE; // for calls below
      switch(cmd){
        case 7:
          clearRow(2);
          lcd.setCursor((20-15)/2,2);
          lcd.print("Motion Detection");
          delay(2000);
          motionSensor.enable();
          break;
        case 8:
          clearRow(2);
          lcd.setCursor((20-23)/2,2);
          lcd.print("Motion Detection OFF");
          delay(2000);
          motionSensor.disable();
          break;
        case 9:
          clearRow(2);
          lcd.setCursor((20-10)/2,2);
          lcd.print("Radio Mode");
          delay(2000);
          fmRadio.enable(true);
          break;
        case 10:
          clearRow(2);
          lcd.setCursor((20-19)/2,2);
          lcd.print("Next Radio Station");
          delay(2000);
          fmRadio.enable(true); fmRadio.next();
          break;
        case 11:
          if(fmRadio.isEnabled()){
            clearRow(2);
            lcd.setCursor((20-15)/2,2);
            lcd.print("Radio Mode Off");
            delay(2000);
            fmRadio.enable(false);
          }
          break;
        case 12:
          clearRow(2);
          lcd.setCursor((20-19)/2,2);
          lcd.print("Bluetooth Mode Exit");
          delay(2000);
          // send BACK to pause
          bluetooth.handle(false,false,false,true,s);
          break;
        case 92:
          clearRow(2);
          lcd.setCursor((20-15)/2,2);
          lcd.print("Playing Music");
          delay(2000);
          // OK toggles play/pause → play
          bluetooth.handle(true,false,false,false,s);
          break;
        case 93:
          clearRow(2);
          lcd.setCursor((20-13)/2,2);
          lcd.print("Music Paused");
          delay(2000);
          // OK toggles → pause
          bluetooth.handle(true,false,false,false,s);
          break;
        case 94:
          clearRow(2);
          lcd.setCursor((20-18)/2,2);
          lcd.print("Playing Last Track");
          delay(2000);
          // LEFT = prev
          bluetooth.handle(false,true,false,false,s);
          break;
        case 95:
          clearRow(2);
          lcd.setCursor((20-18)/2,2);
          lcd.print("Playing Next Track");
          delay(2000);
          // RIGHT = next
          bluetooth.handle(false,false,true,false,s);
          break;
        case 103:
          clearRow(2);
          lcd.setCursor((20-6)/2,2);
          lcd.print("LED On");
          delay(2000);
          ledDimmer.set(100);
          break;
        case 104:
          clearRow(2);
          lcd.setCursor((20-7)/2,2);
          lcd.print("LED Off");
          delay(2000);
          ledDimmer.set(0);
          break;
        case 105:
          if(ledDimmer.get()<100){
            clearRow(2);
            lcd.setCursor((20-16)/2,2);
            lcd.print("LED BRT Increased");
            delay(2000);
            ledDimmer.set(ledDimmer.get()+25);
          }
          break;
        case 106:
          if(ledDimmer.get()>0){
            clearRow(2);
            lcd.setCursor((20-16)/2,2);
            lcd.print("LED BRT Decrease");
            delay(2000);
            ledDimmer.set(ledDimmer.get()>=25?ledDimmer.get()-25:0);
          }
          break;
      }
      screen = MAIN_MENU;
      drawHeader();
      drawMenu();
      drawFooter();
    }
  }
}
