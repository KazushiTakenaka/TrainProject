# Project Overview

This project controls a model train using an ESP32. It uses ESP-NOW for wireless communication to receive commands and control the train's motor, lights, and buzzer.

## Hardware and Pinout

Based on the `train_electric.pdf` schematic.

| Component          | Pin (Schematic) | Pin (Code `main.cpp`) | Status      |
| ------------------ | --------------- | --------------------- | ----------- |
| Motor AIN1         | IO25            | `IN1 = 25`            | ✅ Match      |
| Motor AIN2         | IO26            | `IN2 = 26`            | ✅ Match      |
| Motor BIN1 (Light) | IO16            | `IN3 = 16`            | ✅ Match      |
| Motor BIN2 (Light) | IO15            | `IN4 = 15`            | ✅ Match      |
| Buzzer             | IO22            | `BUZZER = 27`         | ❌ **Mismatch** |
| White LED          | IO21            | `WHITE_LED = 17`      | ❌ **Mismatch** |
| Blue LED           | IO19            | `BLUE_LED = 18`       | ❌ **Mismatch** |
| Battery Sense      | IO35            | `BATTERY = 35`        | ✅ Match      |

**Note:** There are discrepancies in the pin definitions for the buzzer and LEDs between the schematic and the code. The code also uses different resistor values for voltage calculation than the schematic shows.

## Function Summary

### `main.cpp`

- **`setup()`**: Initializes Serial, Wi-Fi, ESP-NOW, and pairs with the controller.
- **`loop()`**: Main operational loop. Reads battery voltage, sends data, and processes received commands to control the train. Handles communication loss.
- **`OnDataRecv(...)`**: Callback for receiving ESP-NOW data.
- **`OnDataSent(...)`**: Callback after sending ESP-NOW data.
- **`getVoltage()`**: Reads and calculates the battery voltage.
- **`initializeLedPins()`**: Configures LED pins and LEDC channels.

### `Train` Class (`Train.h`, `Train.cpp`)

- **`Train(...)`**: Constructor. Initializes pins and LEDC channels for motor, lights, and buzzer.
- **`forward(int speed)`**: Moves the train forward.
- **`backward(int speed)`**: Moves the train backward.
- **`stop()`**: Stops the train.
- **`lightOn(int brightness)`**: Turns the light on.
- **`lightOff()`**: Turns the light off.
- **`buzzerOn()`**: Activates the buzzer.
- **`buzzerOff()`**: Deactivates the buzzer.
- **`playTrainSound()`**: Plays a pre-defined train sound effect.
- **`playSirenSound()`**: Plays a pre-defined siren sound effect.

### `ESPNowManager` Class (`ESPNowManager.h`, `ESPNowManager.cpp`)

- **`ESPNowManager()`**: Constructor.
- **`init()`**: Initializes the ESP-NOW service.
- **`pairDevice(...)`**: Manages pairing with a peer device.
