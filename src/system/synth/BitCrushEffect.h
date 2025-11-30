#pragma once

#include "xdk/xaudio2/xaudio2.h"
class BitCrushEffect {
public:
    struct Params {
        u32 unk0;
        float unk4;
    };

    BitCrushEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(BitCrushEffect::Params const &);

    float unk0;
    int unk4;
    float unk8;
    float unkc;
};
