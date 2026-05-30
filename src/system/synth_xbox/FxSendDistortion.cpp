#include "FxSendDistortion.h"
#include "FxSend.h"
#include "dsp/DistortionEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendDistortion360::FxSendDistortion360() : FxSend360(this) {}

FxSendDistortion360::~FxSendDistortion360() {}

void FxSendDistortion360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendDistortion360::UpdateMix() { UpdateVolumes(); }

void FxSendDistortion360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendDistortion360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DistortionEffect::Params p;
    p.unk0 = mBypass;
    p.unk4 = mDrive;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendDistortion360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<DistortionEffect>());
}
