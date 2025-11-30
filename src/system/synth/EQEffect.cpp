#include "synth/EQEffect.h"
#include "xdk/xaudio2/xaudio2.h"

EQEffect::EQEffect(IXAudioBatchAllocator *) {
    unk38 = false;
    unk54 = false;
    unk74 = false;
    unk0 = 12000.0f;
    unk90 = false;
    unk4 = 0;
    unka8 = false;
    unk8 = 8000.0f;
    unkc = 1000.0f;
    unk10 = 0;
    unk14 = 2000.0f;
    unk18 = 0;
    unk1c = 20000.0f;
    unk20 = 0;
    unk24 = 20.0f;
    unk28 = 0;
    unk2c = 0;
    unk30 = 25.0f;
    unk3c = 0;
    unk40 = 0;
    unk44 = 0;
    unk48 = 0;
    unk4c = 0;
    unk50 = 0;
    unk58 = 0;
    unk5c = 0;
    unk60 = 0;
    unk64 = 0;
    unk68 = 0;
    unk6c = 0;
    unk70 = 0;
    unk78 = 0;
    unk7c = 0;
    unk80 = 0;
    unk84 = 0;
    unk88 = 0;
    unk8c = 0;
    unk94 = 0;
    unk98 = 0;
    unk9c = 0;
    unka0 = 0;
    unka4 = 0;
    unkac = 0;
    unkb0 = 0;
    unkb4 = 0;
    unkb8 = 0;
    unkbc = 0;
    Reset();
}
