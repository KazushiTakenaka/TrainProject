#include "ESPNowManager.h"

ESPNowManager::ESPNowManager() : isPaired(false) {}

bool ESPNowManager::init() {
    // ESP-NOWを初期化します。
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW初期化失敗");
        return false;
    } else {
        Serial.println("ESP-NOW初期化成功");
        delay(500); // 初期化後に少し待機します。
        return true;
    }
}

bool ESPNowManager::pairDevice(const uint8_t *mac_addr, int channel, bool autoChannel) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac_addr, 6);
    peerInfo.channel = autoChannel ? WiFi.channel() : channel; // チャンネルを自動設定または手動設定します。
    peerInfo.encrypt = false;

    // ペアリングを試みます。
    int retryCount = 0; // 再試行回数をカウントします。
    while (esp_now_add_peer(&peerInfo) != ESP_OK && retryCount < 15) {
        Serial.println("ペアリング失敗。再試行...");
        delay(500); // 再試行前に待機します。
        retryCount++;
    }

    if (retryCount >= 15) {
        Serial.println("ペアリングに失敗しました。");
        Serial.println("接続できませんでした。");
        isPaired = false;
    } else {
        Serial.println("ペアリング成功");
        isPaired = true; // ペアリング成功を設定します。
    }

    return isPaired;
}