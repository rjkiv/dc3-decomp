#pragma once
#include "xdk/XAUDIO2.h"

// size 0x4
struct HeadsetXferEffectParams {
    void *unk0;
};

DEFINE_CLSID(HeadsetXferEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, BE);

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
