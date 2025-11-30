#include "synth/WahEffect.h"
#include "os/Debug.h"
#include "xdk/xaudio2/xaudio2.h"

WahEffect::WahEffect(IXAudioBatchAllocator *) {
    unk2c = 96000;
    unk0 = 7;
    unk4 = 1000.0f;
    unk8 = 5000.0f;
    unkc = 1.35f;
    unk10 = 0.3f;
    unk14 = -1.0f;
    unk18 = 0.5f;
    unk1c = 1.0f;
    unk20 = 0.5f;
    unk24 = 0;
    unk28 = 1e+30;
    unk30 = 0;
    unk44 = 0;
    unk48 = 0;
    unk40 = 0;
    unk3c = 0;
    unk38 = 0;
    unk34 = 0;
}

void WahEffect::Reset() {
    unk30 = 0;
    unk38 = 0;
    unk34 = 0;
    unk40 = 0;
    unk3c = 0;
}

void WahEffect::SetParameters(WahEffect::Params const &params) {
    unk0 = params.unk4;
    unk8 = params.unk8;
    unk4 = params.unkc;
    unkc = params.unk10;
    unk10 = params.unk14;
    unk14 = params.unk18;
    unk18 = params.unk1c;
    unk1c = params.unk20;
    unk20 = params.unk24;
}

void WahEffect::Process(float *, int, int numChans) { MILO_ASSERT(numChans <= 2, 0x34); }
