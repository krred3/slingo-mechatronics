#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---- User-defined pins & labels ----
const int buttonPins[5] = {2, 3, 4, 5, 6};
const int modeLeds  [5] = {13, 12, 11, 10,  9};
const char* modeNames[5] = {
  "LED Mode",
  "Speaker Mode",
  "Bluetooth Mode",
  "Radio Mode",
  "Sensor Mode"
};

const int pirPin    = 28;  // PIR output
const int pirLedPin =  8;  // LED for PIR

LiquidCrystal_I2C lcd(0x27, 16, 2);

int currentMode = -1;  // no mode selected yet

void setup() {
  // initialize buttons & mode LEDs
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(modeLeds[i],   OUTPUT);
    digitalWrite(modeLeds[i], LOW);
  }

  // initialize PIR sensor
  pinMode(pirPin,    INPUT);
  pinMode(pirLedPin, OUTPUT);
  digitalWrite(pirLedPin, LOW);

  // initialize LCD
  lcd.begin();      // <— call begin() with no args
  lcd.backlight();
  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  // 1) Check for a mode-button press
  for (int i = 0; i < 5; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      if (currentMode != i) {
        currentMode = i;
        updateModeDisplay();
      }
      delay(200);  // basic debounce
      break;       // only handle one button at a time
    }
  }

  // 2) If we're in Sensor Mode (index 4), read PIR and drive its LED
  if (currentMode == 4) {
    digitalWrite(pirLedPin, digitalRead(pirPin) == HIGH ? HIGH : LOW);
  } else {
    // ensure PIR LED is off in all other modes
    digitalWrite(pirLedPin, LOW);
  }
}

// Helper: turn on the selected mode LED & update the LCD
void updateModeDisplay() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(modeLeds[i], (i == currentMode) ? HIGH : LOW);
  }
  lcd.clear();
  lcd.print(modeNames[currentMode]);
}
