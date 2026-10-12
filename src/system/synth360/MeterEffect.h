#pragma once
#include "synth/FxSend.h"
#include "xdk/XAUDIO2.h"

struct MeterEffectParams {
    LevelData *pLevelData; // 0x0
};

DEFINE_CLSID(MeterEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, CC);

class MeterEffect : public ATG::CSampleXAPOBase<MeterEffect, MeterEffectParams> {
public:
    MeterEffect();

    virtual void OnSetParameters(const MeterEffectParams &);
    virtual void
    DoProcess(const MeterEffectParams &, float *__restrict, unsigned int, unsigned int);

private:
    float mSumSquares[6]; // 0x60
    float mPeaks[6]; // 0x78
    unsigned int mSampleCount; // 0x90
};
