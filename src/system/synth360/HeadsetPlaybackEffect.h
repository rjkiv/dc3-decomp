#pragma once
#include "synth360/HeadsetXferEffect.h"
#include "xdk/XAUDIO2.h"

// size 0x1
struct HeadsetPlaybackEffectParams {
    bool unk0;
};

class __declspec(uuid("B4D4C8AA-A20D-40A1-84A7-64193551A9BF")) HeadsetPlaybackEffect
    : public ATG::CSampleXAPOBase<HeadsetPlaybackEffect, HeadsetPlaybackEffectParams> {
public:
    HeadsetPlaybackEffect(HeadsetXferEffect **);
    virtual ~HeadsetPlaybackEffect() {}
    virtual void DoProcess(
        const HeadsetPlaybackEffectParams &, float *__restrict, unsigned int, unsigned int
    );

private:
    int unk68;
    HeadsetXferEffect *mEffects[4];
};
