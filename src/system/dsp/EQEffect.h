#pragma once
#include "xdk/XAUDIO2.h"

// size 0x34C
class EQEffect {
public:
    struct Params {
        bool unk0;
    };

    EQEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameter(int, float);
    void SetParameters(const EQEffect::Params &);

private:
    float unk0;
    float unk4;
    float unk8;
    float unkc;
    float unk10;
    float unk14;
    float unk18;
    float unk1c;
    float unk20;
    float unk24;
    float unk28;
    float unk2c;
    float unk30;
    u32 unk34;
    bool unk38;
    float unk3c;
    float unk40;
    float unk44;
    float unk48;
    float unk4c;
    float unk50;
    bool unk54;
    float unk58;
    float unk5c;
    float unk60;
    float unk64;
    float unk68;
    float unk6c;
    float unk70;
    bool unk74;
    float unk78;
    float unk7c;
    float unk80;
    float unk84;
    float unk88;
    float unk8c;
    bool unk90;
    float unk94;
    float unk98;
    float unk9c;
    float unka0;
    float unka4;
    bool unka8;
    float unkac;
    float unkb0;
    float unkb4;
    float unkb8;
    float unkbc;
};
