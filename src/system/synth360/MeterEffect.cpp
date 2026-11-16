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

void MeterEffect::DoProcess(
    const MeterEffectParams &params,
    float *__restrict pData,
    unsigned int sampleCount,
    unsigned int channelCount
) {
    for (int i = 0; i < channelCount; i++) {
        for (int j = 0; j < sampleCount; j++) {
            float f1 = pData[i * sampleCount + j];
            mSumSquares[i] += f1 * f1;
            MaxEq(mPeaks[i], fabsf(f1));
        }
        if (params.pLevelData) {
            params.pLevelData[i].mPeak = mPeaks[i];
            mPeaks[i] = 0;
            if (mSampleCount != 0) {
                params.pLevelData[i].mRMS = sqrtf(mSumSquares[i] / (float)mSampleCount);
            }
        }
    }
    mSampleCount += sampleCount;
}
