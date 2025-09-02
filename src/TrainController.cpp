#include "TrainController.h"
#include <Arduino.h>

TrainController::TrainController(Train& train) : train(train), firstStep(0) {}

void TrainController::handleReceivedData(const ReceivedDataPacket& data) {
    // モータ出力操作、前進後進を切り替えます。
    if (data.sld_sw1_1 == 1 && data.sld_sw1_2 == 1) {
        Serial.println("モータ停止");
        train.stop();
    } else if (data.sld_sw1_2 == 0) {
        Serial.println("モータ前進");
        train.forward(data.slideVal1);
    } else if (data.sld_sw1_1 == 0) {
        Serial.println("モータ後進");
        train.backward(data.slideVal1);
    }

    // ライトを操作します。
    if (data.sld_sw2_2 == 1 && data.sld_sw2_1 == 1) {
        train.lightOn(100);
    } else if (data.sld_sw2_2 == 0) {
        // 起動時にライトを消灯させます。
        if (firstStep == 0) {
            train.lightOff();
            // firstStep = 1;
        } else if (firstStep == 1) {
            if (data.sw2 == 1) {
                train.lightOn(255);
            } else if (data.sw2 == 0) {
                train.lightOff();
            }
        }
    } else if (data.sld_sw2_1 == 0) {
        if (data.sw2 == 0) {
            train.lightOn(255);
        } else if (data.sw2 == 1) {
            train.lightOff();
        }
    }

    // ブザーを操作します。
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