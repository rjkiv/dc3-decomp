#pragma once
#include "xdk/XAUDIO2.h"

// size 0x1
struct GainEffectParams {
    bool unk0;
};

class __declspec(uuid("B4D4C8AA-A20D-40A1-84A7-64193551A9BC")) GainEffect
    : public ATG::CSampleXAPOBase<GainEffect, GainEffectParams> {
public:
    GainEffect();

    virtual void
    DoProcess(const GainEffectParams &, float *__restrict, unsigned int, unsigned int);

    static void SetGain(float gain) { sGain = gain; }

private:
    static float sGain;
};
