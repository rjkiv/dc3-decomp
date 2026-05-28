#pragma once
#include "xdk/XAUDIO2.h"

// size 0x10
class BitCrushEffect {
public:
    struct Params {
        Params() : unk0(false) {}
        bool unk0;
        float unk4;
    };

    BitCrushEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(const BitCrushEffect::Params &);
    void Reset();

private:
    float unk0;
    int unk4;
    float unk8;
    float unkc;
};
