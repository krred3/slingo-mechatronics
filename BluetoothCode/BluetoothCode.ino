#include <SoftwareSerial.h>

// ---------- Pin assignments ----------
const uint8_t BTN_PIN = 3;                // push‑button to GND
const uint8_t BT_TX  = 10;                // goes to ESP32 RX2 (via divider)
const uint8_t BT_RX  = 11;                // not used but required by lib

// ---------- Serial ports ----------
SoftwareSerial bt(BT_RX, BT_TX);          // RX, TX  (we only TX)
const unsigned long BAUD_BT   = 9600;     // match ESP32 Serial2 baud
const unsigned long BAUD_USB  = 115200;   // Serial Monitor

void setup() {
  // USB Serial for debug
  Serial.begin(BAUD_USB);
  while (!Serial) {}
  Serial.println(F("\n[NANO] Debug start"));

  // UART to ESP32
  bt.begin(BAUD_BT);
  Serial.println(F("[NANO] SoftwareSerial begun on D10 (TX)"));

  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("[NANO] Button on D2 (active LOW)"));
}

void loop() {
  static bool last = HIGH;
  bool now = digitalRead(BTN_PIN);

  // detect button press (falling edge)
  if (last == HIGH && now == LOW) {
    Serial.println(F("[NANO] Button pressed → sending 'P'"));
    bt.write('P');
  }
  last = now;

  // ---------- echo any bytes coming back (optional) ----------
  if (bt.available()) {
    int c = bt.read();
    Serial.print(F("[NANO] Received from ESP32: 0x"));
    Serial.println(c, HEX);
  }
}
