/*
  TEA5767 FM Radio – Button‑Tuned
  --------------------------------
  • Headphones   → TEA5767 L, R, GND pins
  • TEA5767 SDA  → A3   (soft‑I2C SDA)
  • TEA5767 SCL  → A4   (soft‑I2C SCL)
  • TEA5767 VCC  → 5 V  (3.3–5 V acceptable on most breakouts)
  • TEA5767 GND  → GND
  • Tune‑UP button   → D5  (wired to GND when pressed)
  • Tune‑DOWN button → D6  (wired to GND when pressed)

  Created 2 May 2025 – Kreed’s FM‑only sketch
*/

#include <Wire.h>
#include <TEA5767.h>

TEA5767  radio;
float    freq = 95.2;           // initial frequency (MHz)

const uint8_t BTN_UP   = 5;
const uint8_t BTN_DOWN = 6;

// ──────────────────────────────────────────────────────────────
void printStatus() {
  Serial.print(F("Tuned to: "));
  Serial.print(freq, 1);
  Serial.println(F(" MHz"));
}

// ──────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  /* Initialise I²C on custom pins A3 (SDA) / A4 (SCL).       *
   * If you’re on an Uno/Nano and want the default pins,      *
   * just call Wire.begin() with no arguments.                */
  Wire.begin(A3, A4);

  radio.init(10);          // 10 = default soft‑mute / volume
  radio.setFrequency(freq);

  Serial.println(F("TEA5767 FM radio ready — press buttons to tune."));
  printStatus();
}

// ──────────────────────────────────────────────────────────────
void loop() {
  if (digitalRead(BTN_UP) == LOW) {           // UP pressed
    freq += 0.1;
    if (freq > 108.0) freq = 87.5;            // wrap‑around
    radio.setFrequency(freq);
    printStatus();
    delay(250);                               // debounce
  }

  if (digitalRead(BTN_DOWN) == LOW) {         // DOWN pressed
    freq -= 0.1;
    if (freq < 87.5) freq = 108.0;            // wrap‑around
    radio.setFrequency(freq);
    printStatus();
    delay(250);
  }

  // Nothing else to do; audio is analogue and continuous.
}
