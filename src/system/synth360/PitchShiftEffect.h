#pragma once
#include "synth360/soundtouch/SoundTouch.h"
#include "xdk/XAUDIO2.h"

struct PitchShiftEffectParams {
    float mParamPitchRatio; // 0x0
};

DEFINE_CLSID(PitchShiftEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, BD);

// size 0x70
class PitchShiftEffect
    : public ATG::CSampleXAPOBase<PitchShiftEffect, PitchShiftEffectParams> {
public:
    PitchShiftEffect();
    virtual ~PitchShiftEffect();
    virtual void DoProcess(
        const PitchShiftEffectParams &, float *__restrict, unsigned int, unsigned int
    );

private:
    soundtouch::SoundTouch *mSoundTouch; // 0x60
    bool mStarted; // 0x64
    float mLastPitch; // 0x68
    int mLastChannels; // 0x6c
};
