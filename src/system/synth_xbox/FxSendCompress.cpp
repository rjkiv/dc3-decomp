#include "FxSendCompress.h"
#include "FxSend.h"
#include "dsp/CompressionEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xaudio2/xaudio2.h"

FxSendCompress360::FxSendCompress360() : FxSend360(this) {}

FxSendCompress360::~FxSendCompress360() {}

void FxSendCompress360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendCompress360::UpdateMix() { UpdateVolumes(); }

void FxSendCompress360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendCompress360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    CompressionEffect::Params p;
    p.unk0 = mBypass;
    p.unk4 = mThresholdDB;
    p.unk8 = mRatio;
    p.unkc = mOutputLevel;
    p.unk10 = mAttack;
    p.unk14 = mRelease;
    p.unk18 = mExpRatio;
    p.unk1c = mExpAttack;
    p.unk20 = mExpRelease;
    p.unk24 = mGateThresholdDB;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendCompress360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<CompressionEffect>());
}
