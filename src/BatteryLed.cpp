#include "BatteryLed.h"

BatteryLed::BatteryLed(int ledChannel, int mediumThreshold, int lowThreshold)
    : _ledChannel(ledChannel),
      _mediumThreshold(mediumThreshold),
      _lowThreshold(lowThreshold),
      _blinkMillis(0),
      _ledState(false) {}

bool BatteryLed::update(int batteryValue) {
    unsigned long currentMillis = millis();
    int interval = 0;

    if (batteryValue <= _lowThreshold) {
        interval = 400; // バッテリー残量がlowThreshold以下の場合は400ms間隔で点滅
    } else if (batteryValue <= _mediumThreshold) {
        interval = 800; // バッテリー残量がmediumThreshold以下の場合は800ms間隔で点滅
    }

    if (interval > 0) {
        if (currentMillis - _blinkMillis >= interval) {
            _blinkMillis = currentMillis;
            _ledState = !_ledState;
            ledcWrite(_ledChannel, _ledState ? 255 : 0);
        }
        return true; // バッテリー残量が少ないため、LED処理を実行した
    } else {
        // バッテリー残量が十分な場合はLEDを消灯
        ledcWrite(_ledChannel, 0);
        _ledState = false;
        return false; // バッテリー残量は十分
    }
}
