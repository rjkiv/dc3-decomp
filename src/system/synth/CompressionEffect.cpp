#include "synth/CompressionEffect.h"
#include "math/Decibels.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cmath>

CompressionEffect::CompressionEffect(IXAudioBatchAllocator *) {
    Params params;
    params.unk0 = false;
    unk34 = 1.0f;
    Reset();
    params.unk4 = -6.0f;
    params.unk8 = 1.0f;
    params.unkc = 1.0f;
    params.unk10 = 0.005f;
    params.unk14 = 0.2f;
    params.unk18 = 1.0f;
    params.unk1c = 0.99f;
    params.unk20 = 1.01f;
    params.unk24 = -40.0f;
    SetParameters(params);
}

void CompressionEffect::Reset() {
    unk38 = 1.0f;
    unk3c = 1.0f;
}

void CompressionEffect::SetParameters(CompressionEffect::Params const &params) {
    unk4 = params.unk4;
    unk0 = DbToRatio(unk4);
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unkc = params.unk8;
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk10 = DbToRatio(params.unkc);
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk14 = 1.0f - (float)exp(-1.0f / (params.unk10 * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk18 = 1.0f - (float)exp(-1.0f / (params.unk14 * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk1c = params.unk18;
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk20 = 1.0f - (float)exp(-1.0f / (params.unk1c * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk24 = 1.0f - (float)exp(-1.0f / (params.unk20 * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk28 = params.unk24;
    float ratio = DbToRatio(unk28);
    unk30 = ratio;
    unk2c = ratio;
    unk8 = DbToRatio(unk4 / unkc - unk4);
}
