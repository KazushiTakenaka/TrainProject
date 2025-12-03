/*送信側(オブジェクト)プログラム*/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
// クラスインクルード
#include "Secret.h" // mac addressインクルード
#include "ESPNowManager.h" // ESPNowManager.hをインクルード
#include "Train.h" // Train.hをインクルード
#include "ReceivedDataPacket.h" // ReceivedDataPacket.hをインクルード
#include "SaneDataPacket.h" // SaneDataPacket.hをインクルード

ESPNowManager espNowManager; // ESPNowManagerのインスタンスを生成
// --- GPIOピン定義 ---
const int IN1 = 25;         // モーター1用ピン
const int IN2 = 26;         // モーター1用ピン
const int IN3 = 16;         // フロントLED用ピン
const int IN4 = 15;         // フロントLED用ピン
const int BUZZER = 27;      // ブザー用ピン
const int BATTERY = 35;     // バッテリー電圧測定用ピン (アナログ入力)
const int WHITE_LED = 17;   // 白色LED用ピン
const int BLUE_LED = 18;    // 青色LED用ピン

// --- グローバル定数 ---
const int BATTERY_LOW_THRESHOLD = 330; // バッテリー低下と判断する電圧閾値 (mV * 10)
const int PAIRING_LED_BRIGHTNESS = 50;   // ペアリング中に点灯する青色LEDの明るさ
const int COMMUNICATION_LOST_THRESHOLD = 10; // 通信ロストと判断するカウント数の閾値


// --- LEDCチャンネル定義 (PWM制御用) ---
const int motorChannel1 = 0; // モーター1用LEDCチャンネル
const int motorChannel2 = 1; // モーター1用LEDCチャンネル
const int motorChannel3 = 2; // フロントLED用LEDCチャンネル
const int motorChannel4 = 3; // フロントLED用LEDCチャンネル
const int buzzerChannel = 4; // ブザー用LEDCチャンネル
const int whiteLedChannel = 5; // 白色LED用LEDCチャンネル
const int blueLedChannel = 6;// 青色LED用LEDCチャンネル

/*関数宣言*/
int getVoltage(); // 電源電圧を取得する
void initializeLedPins(); // ピンモード設定
void handleBattery();
void sendEspNowData();
void handleMotorControl(const ReceivedDataPacket& data);
void handleLightControl(const ReceivedDataPacket& data);
void handleBuzzerControl(const ReceivedDataPacket& data);
void handleCommunicationStatus();

// 受信側のMACアドレスを設定
uint8_t receiver_mac[] = {MAC_ADDRESS_BYTE[0], MAC_ADDRESS_BYTE[1], MAC_ADDRESS_BYTE[2], MAC_ADDRESS_BYTE[3], MAC_ADDRESS_BYTE[4], MAC_ADDRESS_BYTE[5]};

int receivedDataLength = 0; // 受信データの長さ

ReceivedDataPacket receivedData; // 受信データを格納する構造体
ReceivedDataPacket beforeReceiveData; // 前回の受信データを格納するための構造体

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
  if (currentMillis - previousMillis < interval_1) {
    return; // インターバルに達していなければ何もしない
  }
  previousMillis = currentMillis;

  handleBattery();

  if (espNowManager.isPaired) {
    ledcWrite(blueLedChannel, PAIRING_LED_BRIGHTNESS);
    sendEspNowData();
    beforeReceiveData = receivedData; // 受信データをバックアップ

    handleMotorControl(receivedData);
    handleLightControl(receivedData);
    handleBuzzerControl(receivedData);
  } else {
    Serial.println("ペアリングされていないため送信できません");
  }
  
  handleCommunicationStatus();
}

/*関数*/

// バッテリー電圧をチェックし、警告LEDを制御する
void handleBattery() {
  battery_value = getVoltage();
  if (battery_value < BATTERY_LOW_THRESHOLD) {
    // バッテリー電圧が閾値未満の場合、白色LEDを点灯して警告します
    ledcWrite(whiteLedChannel, 100); // 白色LEDを最大輝度で点灯
  } else {
    ledcWrite(whiteLedChannel, 0); // 白色LEDを消灯
  }
}

// ESP-NOWでデータを送信する
void sendEspNowData() {
  // 送信データを設定(↓サンプル、他に送りたいデータがあれば変更、追加する)
  sendData.val1 = 1;sendData.val2 = 2;sendData.val3 = 3;sendData.val4 = 4;sendData.val5 = 5;
  esp_err_t result = esp_now_send(receiver_mac, (uint8_t *)&sendData, sizeof(sendData)); // データ送信
  if (result != ESP_OK) {
    Serial.print("送信エラー: ");
    Serial.println(result);
  }
}

// モーターの制御を行う
void handleMotorControl(const ReceivedDataPacket& data) {
  /*モータ出力操作、前進後進切替*/
  if (data.sld_sw1_1 == 1 && data.sld_sw1_2 == 1) {
    train.stop();
  } else if (data.sld_sw1_2 == 0) {
    train.forward(data.slideVal1);
  } else if (data.sld_sw1_1 == 0) {
    train.backward(data.slideVal1);
  }
}

// ライトの制御を行う
void handleLightControl(const ReceivedDataPacket& data) {
  /*ライト操作*/
  // スライドスイッチの状態によって動作モードを決定
  if (data.sld_sw2_1 == 1 && data.sld_sw2_2 == 1) {
    // モード: 中央（スライダーで調光）
    train.lightOn(data.slideVal2);
  } else if (data.sld_sw2_1 == 0) {
    // モード: ポジション1
    if (data.sw2 == 0) {
      train.lightOn(255);
    } else {
      train.lightOff();
    }
  } else if (data.sld_sw2_2 == 0) {
    // モード: ポジション2
    if (data.sw2 == 1) {
      train.lightOn(255);
    } else {
      train.lightOff();
    }
  } else {
    // 上記のどのモードでもない場合（想定外の状態）
    train.lightOff();
  }
}

// ブザーの制御を行う
void handleBuzzerControl(const ReceivedDataPacket& data) {
  /* ブザー操作*/
  if (data.sw1 == 0) {
    if (firstStep == 0) {
      train.buzzerOff();
      firstStep = 1;
    } else if (firstStep == 1) {
      train.buzzerOn();
    }
  } else if (data.sw1 == 1) {
    train.buzzerOff();
  }
}

// 通信状態を監視し、状況に応じた処理を行う
void handleCommunicationStatus() {
  if (receivedDataLength > 0) {
    # if 1
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
    if (lostCount > COMMUNICATION_LOST_THRESHOLD) {
      // ペアリングされていない場合の処理です (ブリージングエフェクト)
      // 2000ms (2秒)周期で明るさを計算します
      float rad = (millis() % 2000) / 2000.0 * 2.0 * PI;
      // sinカーブを使い、0-255の範囲で滑らかな明るさの変化を生成します
      int brightness = (int)((sin(rad - PI / 2.0) + 1.0) / 2.0 * 255);
      ledcWrite(blueLedChannel, brightness);
      Serial.println("通信が途切れています");
      train.stop();
      train.lightOff();
      train.buzzerOff();
    }
  }
}

// ピンモード設定
void initializeLedPins() {
  const int freq = 5000;
  const int resolution = 8;
  ledcSetup(whiteLedChannel, freq, resolution);
  ledcAttachPin(WHITE_LED, 5);
  ledcWrite(whiteLedChannel, 0); // 初期状態は消灯
  ledcSetup(blueLedChannel, freq, resolution);
  ledcAttachPin(BLUE_LED, 6);
  ledcWrite(blueLedChannel, 0); // 初期状態は消灯
}

int getVoltage() {
  int voltage_value = analogRead(BATTERY);
  // 分圧抵抗の値 (実際の回路に合わせてください)
  const int R1 = 10000;
  const int R2 = 20000;

  // 電圧を計算します (ESP32のADCは3.3V基準、12bit分解能(0-4095))
  // 計算式: Vout = Vin * R2 / (R1 + R2) -> Vin = Vout * (R1 + R2) / R2
  // Vout = analogRead値 * 3.3 / 4095.0
  // 注意: ESP32のADC基準電圧は内部で変動することがあるため、より正確な測定にはキャリブレーションが必要です。
  double voltage = (voltage_value * 3.3 / 4095.0) * (double)(R1 + R2) / R2;

  return (int)(voltage * 100);
}

