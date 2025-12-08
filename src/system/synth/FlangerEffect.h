#pragma once
#include "xdk/xaudio2/xaudio2.h"

class FlangerEffect {
public:
    struct Params {
        u32 unk0;
        float unk4;
        float unk8;
        float unkc;
        float unk10;
        float unk14;
    };

    ~FlangerEffect();
    FlangerEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameters(FlangerEffect::Params const &);

    float *unk0[4];
    int unk10;
    int unk14;
    float unk18;
    float unk1c;
    float unk20;
    float unk24;
    float unk28;
    float unk2c;
    float unk30;
};
