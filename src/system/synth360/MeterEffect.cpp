#include "synth360/MeterEffect.h"

MeterEffect::MeterEffect() : mSampleCount(0) {
    for (int i = 0; i < 6; i++) {
        mSumSquares[i] = 0;
        mPeaks[i] = 0;
    }
    MeterEffectParams p;
    p.pLevelData = nullptr;
    SetParameters(&p, sizeof(p));
}

void MeterEffect::OnSetParameters(const MeterEffectParams &params) {
    mSampleCount = 0;
    for (int i = 0; i < 6; i++) {
        mSumSquares[i] = 0;
    }
}
