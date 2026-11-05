#include "synth360/HeadsetPlaybackEffect.h"
#include "synth360/HeadsetXferEffect.h"
#include <cstring>

HeadsetPlaybackEffect::HeadsetPlaybackEffect(HeadsetXferEffect **ptrs) : mFrameNum(0) {
    for (int i = 0; i < 4; i++) {
        mXferEffects[i] = ptrs[i];
    }
    HeadsetPlaybackEffectParams initialParams = {};
    SetParameters(&initialParams, 1);
}

void HeadsetPlaybackEffect::DoProcess(
    const HeadsetPlaybackEffectParams &params,
    float *__restrict pData,
    unsigned int cFrames,
    unsigned int cChannels
) {
    for (int i = 0; i < 4; i++) {
        memcpy(pData, &mXferEffects[(mFrameNum % 2) * 256], sizeof(mXferEffects) * 64);
        pData += 0x100;
    }
    mFrameNum++;
}
