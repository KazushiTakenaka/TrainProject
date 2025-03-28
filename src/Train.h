#ifndef TRAIN_H
#define TRAIN_H

#include <Arduino.h>

class Train {
public:
    Train(int IN1, int IN2, int IN3, int IN4, int buzzer, 
        int motorChannel1, int motorChannel2, int motorChannel3, int motorChannel4, int buzzerChannel);
    void forward(int forward_int);
    void backward(int backward_int);
    void stop();
    void lightOn(int light_int);
    void lightOff();
    void buzzerOn();
    void buzzerOff();
    void playTrainSound();
    void playSirenSound();

private:
    int IN1, IN2, IN3, IN4, BUZZER;
    int motorChannel1, motorChannel2, motorChannel3, motorChannel4, buzzerChannel;
};

#endif // Train_H