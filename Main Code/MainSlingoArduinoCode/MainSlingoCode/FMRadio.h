// FMRadio.h – TEA5767 helper (revised: explicit enable/disable)
#ifndef FM_RADIO_H
#define FM_RADIO_H

#include <Wire.h>
#include <TEA5767.h>

class FMRadioModule {
 public:
  FMRadioModule() : _idx(0), _enabled(false) {}

  /* Call once from setup() – just initialise I²C and the chip */
  void begin() {
    Wire.begin();              // Mega SDA=20, SCL=21
    _radio.init(10);           // soft‑mute/attempt parameter
    _radio.setMuted(true);     // start SILENT
  }

  /* Turn audio on/off (use when entering/leaving FM mode) */
  void enable(bool on) {
    _enabled = on;
    if (on) {
      tune();                  // ensure frequency loaded
      _radio.setMuted(false);  // un‑mute
    } else {
      _radio.setMuted(true);   // quick mute
      /* Optional full standby:
         _radio.setStandBy(true);   // saves ~25 mA
      */
    }
  }

  /* Preset stepping (works only if enabled) */
  void next() { if (_enabled) { _idx = (_idx + 1) % _count; tune(); } }
  void prev() { if (_enabled) { _idx = (_idx + _count - 1) % _count; tune(); } }

  float current() const { return _presets[_idx]; }
  bool  isEnabled() const { return _enabled; }

 private:
  void tune() {
    _radio.setFrequency(_presets[_idx]);
    delay(50);  // allow PLL to lock
  }

  TEA5767 _radio;
  static inline const float _presets[5] = {92.3, 92.7, 93.5, 98.7, 99.5};
  static constexpr uint8_t _count =
      sizeof(_presets)/sizeof(_presets[0]);

  uint8_t _idx;
  bool    _enabled;
};

#endif /* FM_RADIO_H */
