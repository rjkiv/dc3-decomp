#include "FxSendDistortion.h"
#include "FxSend.h"
#include "dsp/DistortionEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendDistortion360::FxSendDistortion360() : FxSend360(this) {}

FxSendDistortion360::~FxSendDistortion360() {}

void FxSendDistortion360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DistortionEffect::Params p;
    p.bypass = mBypass;
    p.drive = mDrive;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendDistortion360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<DistortionEffect>());
}
