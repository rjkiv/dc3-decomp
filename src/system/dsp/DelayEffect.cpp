#include "dsp/DelayEffect.h"
#include "Common_Xbox.h"
#include "math/Decibels.h"
#include "os/Debug.h"

DelayEffect::DelayEffect(IXAudioBatchAllocator *ix)
    : unk0(24000), unk4(0), unk8(0.3f), unkc(0.5f) {
    DspAllocate(unk10, 192000, ix);
}

DelayEffect::~DelayEffect() { DspFree(unk10); }

void DelayEffect::Reset() { DspClearBuffer(unk10, 192000); }

void DelayEffect::SetParameters(const DelayEffect::Params &params) {
    SetParameter(0, params.unk4);
    unk8 = DbToRatio(params.unk8);
    unkc = params.unkc / 100.0f;
}
