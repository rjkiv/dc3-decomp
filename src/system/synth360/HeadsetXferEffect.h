#pragma once
#include "xdk/XAUDIO2.h"

class HeadsetXferEffect;

// size 0x4
struct HeadsetXferEffectParams {
    HeadsetXferEffect *effect; // 0x0
};

DEFINE_CLSID(HeadsetXferEffect, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, BE);

class HeadsetXferEffect
    : public ATG::CSampleXAPOBase<HeadsetXferEffect, HeadsetXferEffectParams> {
public:
    HeadsetXferEffect();
    virtual ~HeadsetXferEffect() {}
    virtual void DoProcess(
        const HeadsetXferEffectParams &params,
        float *pData,
        unsigned int cFrames,
        unsigned int cChannels
    );

private:
    int mFrameNum; // 0x60
    float mData[512]; // 0x64
};
