#include "FxSendCompress.h"
#include "FxSend.h"
#include "dsp/CompressionEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendCompress360::FxSendCompress360() : FxSend360(this) {}

FxSendCompress360::~FxSendCompress360() {}

void FxSendCompress360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    CompressionEffect::Params p;
    p.bypass = mBypass;
    p.thresholdDB = mThresholdDB;
    p.ratio = mRatio;
    p.outputLevel = mOutputLevel;
    p.attack = mAttack;
    p.release = mRelease;
    p.expRatio = mExpRatio;
    p.expAttack = mExpAttack;
    p.expRelease = mExpRelease;
    p.gateThresholdDB = mGateThresholdDB;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendCompress360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<CompressionEffect>());
}
