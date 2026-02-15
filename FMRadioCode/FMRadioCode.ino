#include <Wire.h>
#include <TEA5767.h>

TEA5767 Radio;
float freq = 95.2;

const int buttonUp = 3;
const int buttonDown = 6;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  Radio.init(10);
  Radio.setFrequency(freq);

  Serial.println("TEA5767 FM Radio Ready. Press buttons to tune.");
  printStatus();
}

void loop() {
  if (digitalRead(buttonUp) == LOW) {
    freq += 0.1;
    if (freq > 108.0) freq = 87.5;
    Radio.setFrequency(freq);
    delay(300);
    printStatus();
  }

  if (digitalRead(buttonDown) == LOW) {
    freq -= 0.1;
    if (freq < 87.5) freq = 108.0;
    Radio.setFrequency(freq);
    delay(300);
    printStatus();
  }
}

void printStatus() {
  Serial.print("Tuned to: ");
  Serial.print(freq, 1);
  Serial.println(" MHz");
}
