#pragma once
#include "xdk/XAUDIO2.h"

// size 0x10
class __declspec(uuid("D794C77C-D14D-470C-9346-B9BE9AC4860B")) BitCrushEffect {
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
