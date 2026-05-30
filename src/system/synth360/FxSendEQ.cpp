#include "FxSendEQ.h"
#include "FxSend.h"
#include "dsp/EQEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendEQ360::FxSendEQ360() : FxSend360(this) {}

FxSendEQ360::~FxSendEQ360() {}

void FxSendEQ360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendEQ360::UpdateMix() { UpdateVolumes(); }

void FxSendEQ360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendEQ360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    EQEffect::Params p;
    p.bypass = mBypass;
    p.highFreqCutoff = mHighFreqCutoff;
    p.highFreqGain = mHighFreqGain;
    p.midFreqCutoff = mMidFreqCutoff;
    p.midFreqBandwidth = mMidFreqBandwidth;
    p.midFreqGain = mMidFreqGain;
    p.lowFreqCutoff = mLowFreqCutoff;
    p.lowFreqGain = mLowFreqGain;
    p.highPassCutoff = mHighPassCutoff;
    p.lowPassCutoff = mLowPassCutoff;
    p.lowPassReso = mLowPassReso;
    p.highPassReso = mHighPassReso;
    p.lrMode = mLRMode;
    p.transitionTime = mTransitionTime;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendEQ360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<EQEffect>());
}
