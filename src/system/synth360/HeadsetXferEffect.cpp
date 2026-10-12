#include "synth360/HeadsetXferEffect.h"
#include <cstring>

HeadsetXferEffect::HeadsetXferEffect() : mFrameNum(0) {
    memset(mData, 0, sizeof(mData));
    HeadsetXferEffectParams p;
    p.effect = this;
    SetParameters(&p, sizeof(p));
}

void HeadsetXferEffect::DoProcess(
    const HeadsetXferEffectParams &, float *__restrict farr, unsigned int, unsigned int
) {
    // fix
    memcpy(&mData[mFrameNum % 2], farr, sizeof(mData[mFrameNum % 2]));
    mFrameNum++;
}
