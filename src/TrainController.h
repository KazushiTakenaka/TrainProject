#ifndef TRAINCONTROLLER_H
#define TRAINCONTROLLER_H

#include "Train.h"
#include "ReceivedDataPacket.h"

// Trainの制御を行うクラスです。
class TrainController {
public:
    TrainController(Train& train);
    void handleReceivedData(const ReceivedDataPacket& data);

private:
    Train& train;
    int firstStep;
};

#endif // TRAINCONTROLLER_H