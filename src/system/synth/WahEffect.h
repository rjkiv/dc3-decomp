#pragma once

#include "xdk/xaudio2/xaudio2.h"
class WahEffect {
public:
    struct Params {
        u32 unk0;
        float unk4;
        float unk8;
        float unkc;
        float unk10;
        float unk14;
        float unk18;
        float unk1c;
        bool unk20;
        float unk24;
    };

    WahEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameters(WahEffect::Params const &);

    float unk0;
    float unk4;
    float unk8;
    float unkc;
    float unk10;
    float unk14;
    float unk18;
    float unk1c;
    float unk20;
    float unk24;
    float unk28;
    int unk2c;
    float unk30;
    float unk34;
    float unk38;
    float unk3c;
    float unk40;
    float unk44;
    float unk48;
};
