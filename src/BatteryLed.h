#ifndef BATTERY_LED_H
#define BATTERY_LED_H

#include <Arduino.h>

class BatteryLed {
public:
    BatteryLed(int ledChannel, int mediumThreshold, int lowThreshold);
    // バッテリー残量に応じてLEDを更新します。
    // バッテリー残量が少ない場合はtrueを、そうでない場合はfalseを返します。
    bool update(int batteryValue);

private:
    int _ledChannel;
    int _mediumThreshold;
    int _lowThreshold;
    unsigned long _blinkMillis;
    bool _ledState;
};

#endif // BATTERY_LED_H
