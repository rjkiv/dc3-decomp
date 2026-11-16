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
    // start at either the first or second half of HeadsetXferEffect::mData (float[512])
    int offset = (mFrameNum % 2) * 256;
    for (int i = 0; i < DIM(mXferEffects); i++) {
        // copies either the first or second half of HeadsetXferEffect's mData field
        memcpy(pData, &mXferEffects[i]->mData[offset], 256 * sizeof(float));
        // then advance by that many bytes
        pData += 256;
    }
    mFrameNum++;
}
