#pragma once
#include "xdk/XAUDIO2.h"

// size 0x4
class DistortionEffect {
public:
    struct Params {
        Params() : unk0(0) {}
        bool unk0; // 0x0 - bypass
        float unk4; // 0x4 - amt of distortion drive
    };

    DistortionEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(const DistortionEffect::Params &);
    void Reset();

private:
    float unk0;
};
