// LEDDimmer.h – simple PWM dimmer wrapper
#ifndef LED_DIMMER_H
#define LED_DIMMER_H

#include <Arduino.h>

class LEDDimmerModule {
 public:
  LEDDimmerModule(uint8_t pin,
                  uint8_t minPct = 0,
                  uint8_t maxPct = 100,
                  uint8_t step   = 5)
      : _pin(pin), _min(minPct), _max(maxPct), _step(step), _pct(minPct) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    apply();
  }

  void update() {
    /* nothing time‑critical here, but keep for symmetry */
  }

  void set(uint8_t pct) {
    if (pct < _min) pct = _min;
    if (pct > _max) pct = _max;
    _pct = pct;
    apply();
  }

  void inc(uint8_t amt = 0) { set(_pct + (amt ? amt : _step)); }
  void dec(uint8_t amt = 0) { set(_pct - (amt ? amt : _step)); }

  uint8_t get() const { return _pct; }

 private:
  void apply() {
    uint8_t duty = map(_pct, 0, 100, 0, 255);
    analogWrite(_pin, duty);
  }

  uint8_t _pin;
  uint8_t _min, _max, _step;
  uint8_t _pct;          // current brightness %
};

#endif // LED_DIMMER_H


