#pragma once
#include "xdk/XAUDIO2.h"

// size 0x1
struct GainEffectParams {
    bool unk0;
};

class GainEffect : public ATG::CSampleXAPOBase<GainEffect, GainEffectParams> {
public:
    GainEffect();

    virtual void
    DoProcess(const GainEffectParams &, float *__restrict, unsigned int, unsigned int);

    static void SetGain(float gain) { sGain = gain; }

private:
    static float sGain;
};
