#pragma once
#include "os/Timer.h"

class Profiler {
public:
    Profiler(const char *, int);
    void Start();
    void Stop();

private:
    const char *mName; // 0x0
    Timer mTimer; // 0x8
    float mMin; // 0x38
    float mMax; // 0x3c
    float mSum; // 0x40
    unsigned int mCount; // 0x44
    unsigned int mCountMax; // 0x48
};
