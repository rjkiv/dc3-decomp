#include "synth_xbox/HeadsetPlaybackEffect.h"
#include "synth_xbox/HeadsetXferEffect.h"

HeadsetPlaybackEffect::HeadsetPlaybackEffect(HeadsetXferEffect **fx) : unk68(0) {
    for (int i = 0; i < 4; i++) {
        mEffects[i] = fx[i];
    }
    HeadsetPlaybackEffectParams p;
    p.unk0 = false;
    SetParameters(&p, sizeof(p));
}
