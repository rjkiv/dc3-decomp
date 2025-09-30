#pragma once

#include "os/Timer.h"

class VarTimer : public Timer {
public:
    VarTimer();
    void Start();
    void Stop();
    void Reset(float);
    void SetSpeed(float);
    float Ms();

    float unk30;
    float unk34;
};
