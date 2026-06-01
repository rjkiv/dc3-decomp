#include "FxSendBitCrush.h"
#include "FxSend.h"
#include "dsp/BitCrushEffect.h"
#include "dsp/StandardEffect.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "xdk/XAUDIO2.h"

FxSendBitCrush360::FxSendBitCrush360() : FxSend360(this) {}

FxSendBitCrush360::~FxSendBitCrush360() {}

void FxSendBitCrush360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    BitCrushEffect::Params p;
    p.bypass = mBypass;
    p.amount = mAmount;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendBitCrush360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<BitCrushEffect>());
}
