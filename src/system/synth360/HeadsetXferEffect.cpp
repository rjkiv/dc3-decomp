#include "synth360/HeadsetXferEffect.h"
#include <cstring>

HeadsetXferEffect::HeadsetXferEffect() : unk60(0) {
    memset(unk64, 0, sizeof(unk64));
    HeadsetXferEffectParams p;
    p.unk0 = this;
    SetParameters(&p, sizeof(p));
}

void HeadsetXferEffect::DoProcess(
    const HeadsetXferEffectParams &, float *__restrict farr, unsigned int, unsigned int
) {
    memcpy(unk64[unk60 % 2], farr, sizeof(unk64[unk60 % 2]));
    unk60++;
}
