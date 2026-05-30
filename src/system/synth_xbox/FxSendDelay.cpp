#include "FxSendDelay.h"
#include "FxSend.h"
#include "dsp/DelayEffect.h"
#include "dsp/StandardEffect.h"
#include "synth/Utl.h"

FxSendDelay360::FxSendDelay360() : FxSend360(this) {}

FxSendDelay360::~FxSendDelay360() {}

void FxSendDelay360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendDelay360::UpdateMix() { UpdateVolumes(); }

void FxSendDelay360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendDelay360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DelayEffect::Params p;
    if (mTempoSync) {
        p.delayTime = 1 / CalcRateForTempoSync(mSyncType, mTempo);
    } else {
        p.delayTime = mDelayTime;
    }
    p.bypass = mBypass;
    p.gain = mGain;
    p.pingPongPct = mPingPongPct;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendDelay360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<DelayEffect>());
}
