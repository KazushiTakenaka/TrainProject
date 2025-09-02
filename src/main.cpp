/*送信側(オブジェクト)プログラム*/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
// クラスインクルード
#include "Secret.h" // mac addressインクルード
#include "ESPNowManager.h" // ESPNowManager.hをインクルード
#include "Train.h" // Train.hをインクルード
#include "BatteryLed.h" // BatteryLed.hをインクルード
ESPNowManager espNowManager; // ESPNowManagerのインスタンスを生成
const int IN1 = 25;
const int IN2 = 26;
const int IN3 = 16;
const int IN4 = 15;
const int BUZZER = 27;
const int BATTERY = 35;
const int WHITE_LED = 17;
const int BLUE_LED = 18;
// LEDCのチャンネル番号(0~15)
const int motorChannel1 = 0;
const int motorChannel2 = 1;
const int motorChannel3 = 2;
const int motorChannel4 = 3;
const int buzzerChannel = 4;
const int redLedChannel = 5;
const int blueLedChannel = 6;

/*関数宣言*/
int getVoltage(); // 電源電圧を取得する
void initializeLedPins(); // ピンモード設定

// 受信側のMACアドレスを設定
uint8_t receiver_mac[] = {MAC_ADDRESS_BYTE[0], MAC_ADDRESS_BYTE[1], MAC_ADDRESS_BYTE[2], MAC_ADDRESS_BYTE[3], MAC_ADDRESS_BYTE[4], MAC_ADDRESS_BYTE[5]};

int receivedDataLength = 0; // 受信データの長さ

// 受信データを格納する構造体
struct ReceivedDataPacket {
  int slideVal1;int slideVal2;
  int sld_sw1_1;int sld_sw1_2;int sld_sw2_1;int sld_sw2_2;
  int sld_sw3_1;int sld_sw3_2;int sld_sw4_1;int sld_sw4_2;
  int sw1;int sw2;int sw3;int sw4;int sw5;int sw6;int sw7;int sw8;
};
ReceivedDataPacket receivedData; // 受信データを格納する構造体
ReceivedDataPacket beforeReceiveData; // 前回の受信データを格納するための構造体

// 送信データを格納する構造体(↓サンプル、他に送りたいデータがあれば追加する)
// ※コントローラー側の受信するデータ構造体と同じにする
struct SaneDataPacket {
  int val1;int val2;int val3;int val4;int val5;
};

// データ受信時に呼び出されるコールバック関数
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(ReceivedDataPacket)); // 受信データを構造体にコピー
  receivedDataLength = len; // 受信データの長さを設定
}

// データ送信後に呼び出されるコールバック関数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  # if 0
  Serial.print("送信ステータス:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  # endif
}

void setup() {
  Serial.begin(115200); // シリアル通信を開始
  delay(1000); // シリアルが安定するまで待機
  Serial.println("setup");
  initializeLedPins();
  WiFi.mode(WIFI_STA); // ステーションモードに設定通信相手はアクセスポイント兼ステーションモード
  WiFi.disconnect(); // 既存の接続をクリア
  delay(100);
  // 自分のMACアドレスを表示（デバッグ用）
  Serial.print("My MAC address: ");
  Serial.println(WiFi.macAddress());
  
  // 送信先のMACアドレスを表示
  Serial.printf("Send MAC address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
    receiver_mac[0], receiver_mac[1], receiver_mac[2], 
    receiver_mac[3], receiver_mac[4], receiver_mac[5]);
    
    espNowManager.init(); // ESPNowManagerの初期化
    // コールバック登録
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    espNowManager.pairDevice(receiver_mac); // ペアリング
  }
  
Train train(IN1, IN2, IN3, IN4, BUZZER, motorChannel1, motorChannel2, motorChannel3, motorChannel4, buzzerChannel); // Trainのインスタンスを生成
BatteryLed batteryLed(redLedChannel, 250, 230); // BatteryLedのインスタンスを生成
// 送信するデータの構造体を生成
SaneDataPacket sendData;
int battery_value = 0; // BUTTRY電圧確認用
unsigned long previousMillis = 0; // 前回の時間を記録する変数
const int interval_1 = 20; // 待機時間（ミリ秒）
int switchCount = -1; // スイッチのカウント
int firstStep = 0; // 最初のステップ
int lostCount = 0; // 通信が途切れたかどうか
unsigned long ledBlinkMillis = 0; // LED点滅用のタイマー
bool ledState = false; // LEDの状態

// ---------------------------------------------------------------------------------------------

void loop() {
  unsigned long currentMillis = millis(); // 現在の時間を取得
  // 一定時間ごとに実行する処理
  if (currentMillis - previousMillis >= interval_1) {
    previousMillis = currentMillis; // 前回の時間を更新
    battery_value = getVoltage(); // 電源電圧を取得します。

    // 通信状態でLEDを制御
    bool isWaiting = !espNowManager.isPaired || lostCount > 10;
    if (isWaiting) {
      // 通信待機中は0.5秒ごとに点滅
      if (currentMillis - ledBlinkMillis >= 500) {
        ledBlinkMillis = currentMillis;
        ledState = !ledState;
        ledcWrite(blueLedChannel, ledState ? 255 : 0);
      }
    } else {
      // 通信成功時は点灯
      ledcWrite(blueLedChannel, 255);
      ledState = true;
    }

    /*↓ここからメイン処理↓*/
    if (espNowManager.isPaired) {
      // 送信データを設定(↓サンプル、他に送りたいデータがあれば変更、追加する)
      sendData.val1 = 1;sendData.val2 = 2;sendData.val3 = 3;sendData.val4 = 4;sendData.val5 = 5;
      esp_err_t result = esp_now_send(receiver_mac, (uint8_t *)&sendData, sizeof(sendData)); // データ送信
      if (result == ESP_OK) {
        // Serial.println("送信成功");
      } else {
        Serial.print("送信エラー: ");
        Serial.println(result);
      }
      beforeReceiveData = receivedData; // 受信データをバックアップ
      /*モータ出力操作、前進後進切替*/
      if (receivedData.sld_sw1_1 == 1 && receivedData.sld_sw1_2 == 1) {
        train.stop();
      }else if (receivedData.sld_sw1_2 == 0) {
        train.forward(receivedData.slideVal1);
      }else if (receivedData.sld_sw1_1 == 0) {
        train.backward(receivedData.slideVal1);
      }
      // バッテリー低下時のLED処理
      Serial.print("Battery: ");
      Serial.println(battery_value);
      if (!batteryLed.update(battery_value)) {
        /* ライト操作*/
        if (receivedData.sld_sw2_2 == 1 && receivedData.sld_sw2_1 == 1) {
          train.lightOn(100);
        } else if (receivedData.sld_sw2_2 == 0) {
          // 起動時にライトを消灯させる
          if (firstStep == 0) {
            train.lightOff();
            // firstStep = 1;
          } else if (firstStep == 1) {
            if (receivedData.sw2 == 1) {
              train.lightOn(255);
            } else if (receivedData.sw2 == 0) {
              train.lightOff();
            }
          }
        } else if (receivedData.sld_sw2_1 == 0) {
          if (receivedData.sw2 == 0) {
            train.lightOn(255);
          } else if (receivedData.sw2 == 1) {
            train.lightOff();
          }
        }
      }
      /* ブザー操作*/
      if (receivedData.sw1 == 0) {
        if (firstStep == 0) {
          train.buzzerOff();
          firstStep = 1;
        } else if (firstStep == 1) {
          train.buzzerOn();
        }
      } else if (receivedData.sw1 == 1) {
        train.buzzerOff();
      }
    } else {
    Serial.println("ペアリングされていないため送信できません");
  }
  
  if (receivedDataLength > 0) {
    # if 0
    Serial.print("受信データ: ");
    Serial.print(receivedData.slideVal1);Serial.print(" ");
    Serial.print(receivedData.slideVal2);Serial.print(" ");
    Serial.print(receivedData.sld_sw1_1);Serial.print(" ");
    Serial.print(receivedData.sld_sw1_2);Serial.print(" ");
    Serial.print(receivedData.sld_sw2_1);Serial.print(" ");
    Serial.print(receivedData.sld_sw2_2);Serial.print(" ");
    Serial.print(receivedData.sld_sw3_1);Serial.print(" ");
    Serial.print(receivedData.sld_sw3_2);Serial.print(" ");
    Serial.print(receivedData.sld_sw4_1);Serial.print(" ");
    Serial.print(receivedData.sld_sw4_2);Serial.print(" ");
    Serial.print(receivedData.sw1);Serial.print(" ");
    Serial.print(receivedData.sw2);Serial.print(" ");
    Serial.print(receivedData.sw3);Serial.print(" ");
    Serial.print(receivedData.sw4);Serial.print(" ");
    Serial.print(receivedData.sw5);Serial.print(" ");
    Serial.print(receivedData.sw6);Serial.print(" ");
    Serial.print(receivedData.sw7);Serial.print(" ");
    Serial.println(receivedData.sw8);
    # endif
    receivedDataLength = 0; // 受信データの長さをリセット
    lostCount = 0; // 通信が途切れていないのでカウントをリセット
  } else {
    lostCount++; // 通信が途切れているのでカウントを増やす
    if (lostCount > 10) {
      Serial.println("通信が途切れています");
      train.stop();
      train.lightOff();
      train.buzzerOff();
    }
  }
}
}

/*関数*/
// ピンモード設定
void initializeLedPins() {
  const int freq = 5000;
  const int resolution = 8;
  ledcSetup(redLedChannel, freq, resolution);
  ledcAttachPin(WHITE_LED, 5);
  ledcSetup(blueLedChannel, freq, resolution);
  ledcAttachPin(BLUE_LED, 6);
}

// 電源電圧を取得する関数です。
int getVoltage() {
  int voltage_value = analogRead(BATTERY);

  // ADCの読み取り値を電圧に変換し、100倍して整数で返します。
  int voltage = (voltage_value * 3.3 / 4095.0) * 100;
  // テストコード
  // voltage = 240;

  return voltage;
}

