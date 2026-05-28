#pragma once

#include "xdk/xaudio2/xaudio2.h"
class DistortionEffect {
public:
    struct Params {
        u32 unk0;
        float unk4;
    };

    DistortionEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(DistortionEffect::Params const &);

    float unk0;
};
