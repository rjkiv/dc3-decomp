#pragma once
#include "xdk/XAUDIO2.h"

// size 0x34
class FlangerEffect {
public:
    struct Params {
        Params() : unk0(0) {}
        bool unk0;
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
    void SetParameters(const FlangerEffect::Params &);

private:
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
