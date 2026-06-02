#include "FxSendChorus.h"
#include "FxSend.h"
#include "dsp/FlangerEffect.h"
#include "dsp/StandardEffect.h"
#include "synth/Utl.h"

FxSendChorus360::FxSendChorus360() : FxSend360(this) {}

FxSendChorus360::~FxSendChorus360() {}

void FxSendChorus360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    FlangerEffect::Params p;
    p.delayMs = mDelayMs;
    float rate = mRate;
    p.rate = mTempoSync ? CalcRateForTempoSync(mSyncType, mTempo) : rate;
    float transpose = CalcTransposeFromSpeed(p.rate * mDelayMs * 0.0062831854f + 1);
    p.feedbackPct = mFeedbackPct;
    p.bypass = mBypass;
    p.offsetPct = mOffsetPct;
    p.depthPct = Clamp(0.0f, 100.0f, (mDepth / (transpose * 100)) * 100);
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendChorus360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<FlangerEffect>());
}
