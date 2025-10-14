#pragma once

#include "os/Timer.h"

class Profiler {
public:
    Profiler(char const *, int);
    void Start();
    void Stop();

    char const *mName;
    Timer mTimer; // 0x8
    float mMin;
    float mMax;
    float mSum;
    int mCount;
    int mCountMax;
};
