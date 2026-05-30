#pragma once
#include "synth360/HeadsetXferEffect.h"
#include "xdk/XAUDIO2.h"

// size 0x1
struct HeadsetPlaybackEffectParams {
    bool unk0;
};

class HeadsetPlaybackEffect
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
