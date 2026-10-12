#include "synth360/HeadsetPlaybackEffect.h"
#include "synth360/HeadsetXferEffect.h"

HeadsetPlaybackEffect::HeadsetPlaybackEffect(HeadsetXferEffect **fx) : mFrameNum(0) {
    for (int i = 0; i < 4; i++) {
        mXferEffects[i] = fx[i];
    }
    HeadsetPlaybackEffectParams p;
    p.unk0 = false;
    SetParameters(&p, sizeof(p));
}
