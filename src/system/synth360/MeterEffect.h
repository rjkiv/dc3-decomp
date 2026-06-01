#pragma once
#include "xdk/XAUDIO2.h"

struct MeterEffectParams {
    void *unk0;
};

DEFINE_CLSID(MeterEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, CC);

class MeterEffect : public ATG::CSampleXAPOBase<MeterEffect, MeterEffectParams> {
public:
    MeterEffect();

    virtual void OnSetParameters(const MeterEffectParams &);
    virtual void
    DoProcess(const MeterEffectParams &, float *__restrict, unsigned int, unsigned int);

private:
    float unk60[6]; // 0x60
    float unk78[6]; // 0x78
    unsigned int unk90; // 0x90
};
