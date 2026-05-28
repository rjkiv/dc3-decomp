#pragma once
#include "xdk/XAUDIO2.h"

// size 0x50
class WahEffect {
public:
    struct Params {
        bool unk0;
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
    void SetParameters(const WahEffect::Params &);

private:
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
