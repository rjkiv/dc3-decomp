#include "dsp/FlangerEffect.h"
#include "Common_Xbox.h"
#include "math/Rot.h"
#include "xdk/xaudio2/xaudio2.h"

FlangerEffect::FlangerEffect(IXAudioBatchAllocator *ix)
    : mWritePos(0), mDelaySamples(100), mDepth(0), mLastDepth(0), mFeedback(0.5f), mLastPhase(0), mPhaseInc(0), mLastPhaseInc(0),
      mOffset(0.1f) {
    for (int i = 0; i < 2; i++) {
        DspAllocate(mDelayBuffer[i], 0x2580, ix);
        DspAllocate(mDelayBuffer[i + 2], 0x2580, ix);
    }
}

FlangerEffect::~FlangerEffect() {
    for (int i = 0; i < 2; i++) {
        DspFree(mDelayBuffer[i]);
        DspFree(mDelayBuffer[i + 2]);
    }
}

void FlangerEffect::Reset() {
    mWritePos = 0;
    mLastDepth = 0;
    mLastPhase = 0;
    mLastPhaseInc = 0;
    for (int i = 0; i < 2; i++) {
        DspClearBuffer(mDelayBuffer[i], 0x2580);
        DspClearBuffer(mDelayBuffer[i + 2], 0x2580);
    }
}

void FlangerEffect::SetParameters(FlangerEffect::Params const &params) {
    mDelaySamples = params.delayMs * 48.0f;
    mPhaseInc = (params.rate / 48000.0f) * (2 * PI);
    mDepth = params.depthPct / 100.0f;
    mFeedback = params.feedbackPct / 100.0f;
    mOffset = params.offsetPct / 100.0f;
}
