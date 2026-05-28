#pragma once
#include "xdk/XAUDIO2.h"

// size 0x14
class DelayEffect {
public:
    struct Params {
        bool unk0;
        float unk4;
        float unk8;
        float unkc;
    };

    DelayEffect(IXAudioBatchAllocator *);
    ~DelayEffect();
    void Reset();
    void Process(float *, int, int);
    void SetParameter(int, float);
    void SetParameters(const DelayEffect::Params &);

private:
    int unk0;
    int unk4;
    float unk8;
    float unkc;
    float *unk10;
};
