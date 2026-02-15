// MotionSensor.h – self‑contained class for the PIR + LED pair
#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

#include <Arduino.h>

class MotionSensorModule {
 public:
  MotionSensorModule(uint8_t pirPin, uint8_t ledPin)
      : _pirPin(pirPin), _ledPin(ledPin), _enabled(false) {}

  // call once from setup()
  void begin() {
    pinMode(_pirPin, INPUT);
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
  }

  // runtime control
  void enable()  { _enabled = true;  }
  void disable() { _enabled = false; digitalWrite(_ledPin, LOW); }
  void toggle()  { _enabled ? disable() : enable(); }
  bool isEnabled() const { return _enabled; }

  // call every loop(); drives LED whenever module is enabled
  void update() {
    if (_enabled) {
      digitalWrite(_ledPin, digitalRead(_pirPin));
    }
  }

 private:
  uint8_t _pirPin;
  uint8_t _ledPin;
  bool    _enabled;
};

#endif // MOTIONSENSOR_H
