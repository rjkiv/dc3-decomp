#pragma once
#include "xdk/xaudio2/xaudio2.h"

class DelayEffect {
public:
    struct Params {
        u32 unk0;
        float unk4;
        float unk8;
        float unkc;
    };

    ~DelayEffect();
    DelayEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameter(int, float);
    void SetParameters(DelayEffect::Params const &);

    int unk0;
    int unk4;
    float unk8;
    float unkc;
    float *unk10;
};
