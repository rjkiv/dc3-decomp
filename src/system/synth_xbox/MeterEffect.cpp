#include "synth_xbox/MeterEffect.h"

MeterEffect::MeterEffect() : unk90(0) {
    for (int i = 0; i < 6; i++) {
        unk60[i] = 0;
        unk78[i] = 0;
    }
    MeterEffectParams p;
    p.unk0 = 0;
    SetParameters(&p, sizeof(p));
}

void MeterEffect::OnSetParameters(const MeterEffectParams &params) {
    unk90 = 0;
    for (int i = 0; i < 6; i++) {
        unk60[i] = 0;
    }
}
