#pragma once

#include "os/Timer.h"

class VarTimer {
public:
    VarTimer();
    void Start();
    void Stop();
    void Reset(float);
    void SetSpeed(float);
    float Ms();

    Timer mRawTimer; // 0x0
    float mAccumMs; // 0x30
    float mSpeed; // 0x34
};
