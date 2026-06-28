#pragma once

class DeJitter {
public:
    DeJitter();
    void Reset();
    float NewMs(float ms, float &dt);

    static float sTimeScale;

protected:
    float mBuffer[32]; // 0x0
    int mIndex; // 0x80
    int mWindow; // 0x84
    float mLastAverage; // 0x88
    float mLastMs; // 0x8c
};
