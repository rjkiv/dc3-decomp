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
    float *endpoint = samples + data_count;
    if (samples >= endpoint)
        return;
    int i = 0;
    uint pass_count =
        reinterpret_cast<char *>(endpoint) - reinterpret_cast<char *>(samples) - 1;
    pass_count /= sizeof(XMVECTOR) * 4;
    pass_count++;
    do { // why like this...?
        XMVECTOR v0 = __lvx(samples_simd + i, 0x00);
        XMVECTOR v1 = __lvx(samples_simd + i, 0x10);
        XMVECTOR v2 = __lvx(samples_simd + i, 0x20);
        XMVECTOR v3 = __lvx(samples_simd + i, 0x30);
        v0 = __vmulfp(v0, gain);
        v1 = __vmulfp(v1, gain);
        v2 = __vmulfp(v2, gain);
        v3 = __vmulfp(v3, gain);
        __stvx(v0, samples_simd + i, 0x00);
        __stvx(v1, samples_simd + i, 0x10);
        __stvx(v2, samples_simd + i, 0x20);
        __stvx(v3, samples_simd + i, 0x30);
        i += 4;
    } while (--pass_count);
}
