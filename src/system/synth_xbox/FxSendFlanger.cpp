#include "synth_xbox/FxSendFlanger.h"
#include "FxSend.h"
#include "dsp/FlangerEffect.h"
#include "dsp/StandardEffect.h"
#include "synth/Utl.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendFlanger360::FxSendFlanger360() : FxSend360(this) {}

FxSendFlanger360::~FxSendFlanger360() {}

void FxSendFlanger360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendFlanger360::UpdateMix() { UpdateVolumes(); }

void FxSendFlanger360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendFlanger360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    FlangerEffect::Params p;
    p.delayMs = mDelayMs;
    if (mTempoSync) {
        p.rate = CalcRateForTempoSync(mSyncType, mTempo);
    } else {
        p.rate = mRate;
    }
    p.bypass = mBypass;
    p.depthPct = mDepthPct;
    p.feedbackPct = mFeedbackPct;
    p.offsetPct = mOffsetPct;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendFlanger360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<FlangerEffect>());
}
