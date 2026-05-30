#include "dsp/CompressionEffect.h"
#include "math/Decibels.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cmath>

CompressionEffect::CompressionEffect(IXAudioBatchAllocator *) {
    Params params;
    params.bypass = false;
    unk34 = 1.0f;
    Reset();
    params.thresholdDB = -6.0f;
    params.ratio = 1.0f;
    params.outputLevel = 1.0f;
    params.attack = 0.005f;
    params.release = 0.2f;
    params.expRatio = 1.0f;
    params.expAttack = 0.99f;
    params.expRelease = 1.01f;
    params.gateThresholdDB = -40.0f;
    SetParameters(params);
}

void CompressionEffect::Reset() {
    unk38 = 1.0f;
    unk3c = 1.0f;
}

void CompressionEffect::SetParameters(const CompressionEffect::Params &params) {
    unk4 = params.thresholdDB;
    unk0 = DbToRatio(unk4);
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unkc = params.ratio;
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk10 = DbToRatio(params.outputLevel);
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk14 = 1.0f - (float)exp(-1.0f / (params.attack * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk18 = 1.0f - (float)exp(-1.0f / (params.release * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk1c = params.expRatio;
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk20 = 1.0f - (float)exp(-1.0f / (params.expAttack * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk24 = 1.0f - (float)exp(-1.0f / (params.expRelease * 48000.0f));
    unk8 = DbToRatio(unk4 / unkc - unk4);
    unk28 = params.gateThresholdDB;
    float ratio = DbToRatio(unk28);
    unk30 = ratio;
    unk2c = ratio;
    unk8 = DbToRatio(unk4 / unkc - unk4);
}
