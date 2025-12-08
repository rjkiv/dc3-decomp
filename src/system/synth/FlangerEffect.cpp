#include "synth/FlangerEffect.h"
#include "Common_Xbox.h"
#include "math/Rot.h"
#include "xdk/xaudio2/xaudio2.h"

FlangerEffect::FlangerEffect(IXAudioBatchAllocator *ix)
    : unk10(0), unk14(100), unk18(0), unk1c(0), unk20(0.5f), unk24(0), unk28(0), unk2c(0),
      unk30(0.1f) {
    for (int i = 0; i < 2; i++) {
        DspAllocate(unk0[i], 0x2580, ix);
        DspAllocate(unk0[i + 2], 0x2580, ix);
    }
}

FlangerEffect::~FlangerEffect() {
    for (int i = 0; i < 2; i++) {
        DspFree(unk0[i]);
        DspFree(unk0[i + 2]);
    }
}

void FlangerEffect::Reset() {
    unk10 = 0;
    unk1c = 0;
    unk24 = 0;
    unk2c = 0;
    for (int i = 0; i < 2; i++) {
        DspClearBuffer(unk0[i], 0x2580);
        DspClearBuffer(unk0[i + 2], 0x2580);
    }
}

void FlangerEffect::SetParameters(FlangerEffect::Params const &params) {
    unk14 = params.unk4 * 48.0f;
    unk28 = (params.unk8 * 48000.0f) * (2 * PI);
    unk18 = params.unkc / 100.0f;
    unk20 = params.unk10 / 100.0f;
    unk30 = params.unk14 / 100.0f;
}
