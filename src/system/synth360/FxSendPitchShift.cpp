#include "synth360/FxSendPitchShift.h"
#include "synth360/PitchShiftEffect.h"

void FxSendPitchShift360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    PitchShiftEffectParams p;
    p.unk0 = mRatio;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendPitchShift360::CreateFx() {
    return static_cast<CXAPOBase *>(new PitchShiftEffect());
}
