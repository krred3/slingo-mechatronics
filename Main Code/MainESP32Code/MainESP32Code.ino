// ESP32_BT_Receiver.ino
// — receives 1‑byte commands from Mega (Serial2 RX=16, TX=17)
// — streams A2DP sink and sends back track title & time

#include <AudioTools.h>
#include <BluetoothA2DPSink.h>

I2SStream out;
BluetoothA2DPSink a2dp(out);

bool playing = true;
int8_t volume = 50;  // 0..102 range

void setup() {
  // RX2 = GPIO16, TX2 = GPIO17
  Serial2.begin(9600, SERIAL_8N1, /*rxPin=*/16, /*txPin=*/17);

  a2dp.start("SlingoSpeaker");
  a2dp.set_volume(volume);
  a2dp.pause();  // start paused
  playing = false;

  // AVRCP metadata callback: send track title
  a2dp.register_avrcp_metadata_callback(
    [](const char* key, const char* value) {
      if (strcmp(key, BT_AVRCP_METADATA_TRACK) == 0) {
        Serial2.write('T');        // title marker
        Serial2.print(value);      // full title
        Serial2.write('\n');
      }
    }
  );

  // AVRCP event callback: send play‐status (position/total)
  a2dp.register_avrcp_event_callback(
    [](esp_avrc_ct_event_t evt, esp_avrc_rn_param_t* param) {
      if (evt == ESP_AVRC_CT_PLAY_STATUS_RSP) {
        uint32_t pos = param->play_status.track;  // ms
        uint32_t dur = param->play_status.total;  // ms
        Serial2.write('t');                       // time marker
        // send seconds/seconds
        Serial2.printf("%u/%u\n", pos/1000, dur/1000);
      }
    }
  );
}

void loop() {
  if (Serial2.available()) {
    char c = Serial2.read();
    switch (c) {
      case 'M': // enter BT mode
        break;
      case 'E': // exit BT mode
        a2dp.pause();
        playing = false;
        break;
      case 'P': // toggle play/pause
        if (playing) {
          a2dp.pause();
          playing = false;
        } else {
          a2dp.play();
          playing = true;
        }
        break;
      case 'S': // stop
        a2dp.pause();
        playing = false;
        break;
      case 'N': // next track
        a2dp.next();
        break;
      case 'B': // previous track
        a2dp.previous();
        break;
      case '+': // volume up
        if (volume < 100) volume += 5;
        a2dp.set_volume(volume);
        break;
      case '-': // volume down
        if (volume > 0) volume -= 5;
        a2dp.set_volume(volume);
        break;
    }
  }

  delay(10);
}
