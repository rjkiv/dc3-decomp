#pragma once
#include "synth360/soundtouch/SoundTouch.h"
#include "xdk/XAUDIO2.h"

struct PitchShiftEffectParams {
    float unk0;
};

// size 0x70
class __declspec(uuid("B4D4C8AA-A20D-40A1-84A7-64193551A9BD")) PitchShiftEffect
    : public ATG::CSampleXAPOBase<PitchShiftEffect, PitchShiftEffectParams> {
public:
    PitchShiftEffect();
    virtual ~PitchShiftEffect();
    virtual void DoProcess(
        const PitchShiftEffectParams &, float *__restrict, unsigned int, unsigned int
    );

private:
    soundtouch::SoundTouch *mSoundTouch; // 0x60
    int unk64;
    float unk68;
    int unk6c; // 0x6c - num channels
};
