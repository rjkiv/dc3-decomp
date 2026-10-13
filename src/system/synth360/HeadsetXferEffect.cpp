#include "synth360/HeadsetXferEffect.h"
#include <cstring>

HeadsetXferEffect::HeadsetXferEffect() : mFrameNum(0) {
    memset(mData, 0, sizeof(mData));
    HeadsetXferEffectParams p;
    p.effect = this;
    SetParameters(&p, sizeof(p));
}

void HeadsetXferEffect::DoProcess(
    const HeadsetXferEffectParams &params,
    float *__restrict pData,
    unsigned int cFrames,
    unsigned int cChannels
) {
    // fix
    memcpy(&mData[mFrameNum % 2], pData, sizeof(mData[mFrameNum % 2]));
    mFrameNum++;
}
