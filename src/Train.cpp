#include "Train.h"

// タケレールのモーター、ライト、ブザーの制御をします。
Train::Train(int in1, int in2, int in3, int in4, int buzzer, int motorChannel1, int motorChannel2, int motorChannel3, int motorChannel4, int buzzerChannel)
    : IN1(in1), IN2(in2), IN3(in3), IN4(in4), BUZZER(buzzer), motorChannel1(motorChannel1), motorChannel2(motorChannel2), motorChannel3(motorChannel3), motorChannel4(motorChannel4), buzzerChannel(buzzerChannel) {
    const int freq = 5000; // 5kHzのPWM周波数を設定します。
    const int resolution = 8; // 8bitの分解能を設定します。
    ledcSetup(motorChannel1, freq, resolution);
    ledcAttachPin(IN1, motorChannel1);
    ledcSetup(motorChannel2, freq, resolution);
    ledcAttachPin(IN2, motorChannel2);
    ledcSetup(motorChannel3, freq, resolution);
    ledcAttachPin(IN3, motorChannel3);
    ledcSetup(motorChannel4, freq, resolution);
    ledcAttachPin(IN4, motorChannel4);
    ledcSetup(buzzerChannel, freq, resolution);
    ledcAttachPin(BUZZER, buzzerChannel);
}

// 前進させます。
void Train::forward(int ad_int) {
    ledcWrite(motorChannel1, 0);
    ledcWrite(motorChannel2, ad_int);
}

// 後進させます。
void Train::backward(int bk_int) {
    ledcWrite(motorChannel1, bk_int);
    ledcWrite(motorChannel2, bk_int);
}

// 停止させます。
void Train::stop() {
    ledcWrite(motorChannel1, 0);
    ledcWrite(motorChannel2, 0);
}

// ライトを点灯させます。
void Train::lightOn(int light_int) {
    ledcWrite(motorChannel3, 0);
    ledcWrite(motorChannel4, light_int);
}

// ライトを消灯させます。
void Train::lightOff() {
    ledcWrite(motorChannel3, 0);
    ledcWrite(motorChannel4, 0);
}

// ブザーを鳴らします。
void Train::buzzerOn() {
    ledcWriteTone(buzzerChannel, 1000);
}

// ブザーを止めます。
void Train::buzzerOff() {
    ledcWrite(buzzerChannel, 0);
}

// 電車の走行音を再現します。
void Train::playTrainSound() {
    const int gatangFrequency = 128; // ガタンの音の周波数を設定します。（256段階中）
    const int gotonFrequency = 96;   // ゴトンの音の周波数を設定します。（256段階中）
    const int soundDuration = 200;   // 音の持続時間を設定します。（ミリ秒）
    const int pauseDuration = 100;   // 音の間の休止時間を設定します。（ミリ秒）

    for (int i = 0; i < 10; i++) { // 10回繰り返します。
        // ガタンの音
        ledcWriteTone(buzzerChannel, gatangFrequency);
        delay(soundDuration);
        ledcWriteTone(buzzerChannel, 0);
        delay(pauseDuration);

        // ゴトンの音
        ledcWriteTone(buzzerChannel, gotonFrequency);
        delay(soundDuration);
        ledcWriteTone(buzzerChannel, 0);
        delay(pauseDuration);
    }
}

// サイレン音を再生します。
void Train::playSirenSound() {
    const int highFrequency = 1000; // 高い音の周波数を設定します。（256段階中）
    const int lowFrequency = 50;  // 低い音の周波数を設定します。（256段階中）
    const int soundDuration = 300; // 音の持続時間を設定します。（ミリ秒）

    for (int i = 0; i < 10; i++) { // 10回繰り返します。
        // 高い音
        ledcWriteTone(buzzerChannel, highFrequency);
        delay(soundDuration);

        // 低い音
        ledcWriteTone(buzzerChannel, lowFrequency);
        delay(soundDuration);
    }
}