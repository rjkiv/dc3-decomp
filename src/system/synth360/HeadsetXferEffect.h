#pragma once
#include "xdk/XAUDIO2.h"

// size 0x4
struct HeadsetXferEffectParams {
    void *unk0;
};

class __declspec(uuid("B4D4C8AA-A20D-40A1-84A7-64193551A9BE")) HeadsetXferEffect
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
