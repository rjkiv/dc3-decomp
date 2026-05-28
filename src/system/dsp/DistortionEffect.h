#pragma once
#include "xdk/XAUDIO2.h"

// size 0x4
class DistortionEffect {
public:
    struct Params {
        bool unk0;
        float unk4;
    };

    DistortionEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(const DistortionEffect::Params &);

private:
    float unk0;
};
