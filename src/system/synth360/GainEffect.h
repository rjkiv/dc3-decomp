#pragma once
#include "xdk/XAUDIO2.h"

// size 0x1
struct GainEffectParams {
    bool unk0;
};

DEFINE_CLSID(GainEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, BC);

class GainEffect : public ATG::CSampleXAPOBase<GainEffect, GainEffectParams> {
public:
    GainEffect();

    virtual void
    DoProcess(const GainEffectParams &, float *__restrict, unsigned int, unsigned int);

    static void SetGain(float gain) { sGain = gain; }

private:
    static float sGain;
};
