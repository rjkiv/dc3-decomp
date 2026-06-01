#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(BitCrushEffect, D794C77C, D14D, 470C, 93, 46, B9, BE, 9A, C4, 86, 0B);

// size 0x10
class BitCrushEffect {
public:
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        float amount; // 0x4
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
