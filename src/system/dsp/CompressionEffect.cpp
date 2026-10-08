#include "dsp/CompressionEffect.h"
#include "math/Decibels.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cmath>

CompressionEffect::CompressionEffect(IXAudioBatchAllocator *) {
    Params params;
    params.bypass = false;
    mClipAttenuation = 1.0f;
    Reset();
    params.thresholdDB = -6.0f;
    params.ratio = 1.0f;
    params.outputLevel = 1.0f;
    params.attack = 0.005f;
    params.release = 0.2f;
    params.expRatio = 1.0f;
    params.expAttack = 0.99f;
    params.expRelease = 1.01f;
    params.gateThresholdDB = -40.0f;
    SetParameters(params);
}

void CompressionEffect::Reset() {
    mEnvState = 1.0f;
    mExpEnvState = 1.0f;
}

void CompressionEffect::SetParameters(const CompressionEffect::Params &params) {
    mThresholdDB = params.thresholdDB;
    mThreshold = DbToRatio(mThresholdDB);
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mCompRatio = params.ratio;
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mOutputLevel = DbToRatio(params.outputLevel);
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mAttackDelta = 1.0f - (float)exp(-1.0f / (params.attack * 48000.0f));
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mReleaseDelta = 1.0f - (float)exp(-1.0f / (params.release * 48000.0f));
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mExpRatio = params.expRatio;
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mExpAttackDelta = 1.0f - (float)exp(-1.0f / (params.expAttack * 48000.0f));
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mExpReleaseDelta = 1.0f - (float)exp(-1.0f / (params.expRelease * 48000.0f));
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
    mGateThresholdDB = params.gateThresholdDB;
    float ratio = DbToRatio(mGateThresholdDB);
    mGateThreshold = ratio;
    mDynamicGateThreshold = ratio;
    mMakeupGain = DbToRatio(mThresholdDB / mCompRatio - mThresholdDB);
}
