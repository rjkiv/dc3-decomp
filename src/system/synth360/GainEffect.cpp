#include "synth360/GainEffect.h"
#include "vectorintrinsics.h"
#include <cstddef>
#include <cstring>

float GainEffect::sGain = 1;

GainEffect::GainEffect() {
    GainEffectParams p;
    SetParameters(&p, sizeof(p));
}

void GainEffect::DoProcess(
    const GainEffectParams &, float *samples, uint samplect, uint channelct
) {
    uint data_count = samplect * channelct;
    XMVECTOR *samples_simd = reinterpret_cast<XMVECTOR *>(samples);
    XMVECTOR gain;
    gain.v[0] = sGain;
    for (int i = 0; i < 3; i++) {
        gain.u[i + 1] = gain.u[i];
    }
    XMVECTOR *endpoint = reinterpret_cast<XMVECTOR *>(samples + data_count);
    for (; samples_simd < endpoint; samples_simd += 4) {
        samples_simd[0] = __vmulfp(samples_simd[0], gain);
        samples_simd[1] = __vmulfp(samples_simd[1], gain);
        samples_simd[2] = __vmulfp(samples_simd[2], gain);
        samples_simd[3] = __vmulfp(samples_simd[3], gain);
    }
}
