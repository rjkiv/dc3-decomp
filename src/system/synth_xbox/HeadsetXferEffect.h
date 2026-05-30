#pragma once
#include "xdk/XAUDIO2.h"

// size 0x4
struct HeadsetXferEffectParams {
    void *unk0;
};

class HeadsetXferEffect
    : public ATG::CSampleXAPOBase<HeadsetXferEffect, HeadsetXferEffectParams> {
public:
    HeadsetXferEffect();
    virtual ~HeadsetXferEffect() {}
    virtual void DoProcess(
        const HeadsetXferEffectParams &, float *__restrict, unsigned int, unsigned int
    );

private:
    int unk60;
    float unk64[2][0x100];
};
