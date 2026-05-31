#include "dsp/DistortionEffect.h"
#include "math/Utl.h"
#include "os/Debug.h"

DistortionEffect::DistortionEffect(IXAudioBatchAllocator *) : unk0(0) {}

void DistortionEffect::Process(float *samples, int sampct, int numChans) {
    MILO_ASSERT(numChans <= 2, 27);
    float first_peak = 1.0f - unk0; // unk0 some clipping cutoff?
    float min = 0.01f;
    float &p = first_peak < min ? min : first_peak;
    float clipconst = unk0 / p * 2.0f;
    if (sampct <= 0)
        return;
    float cc2 = clipconst + 1.0f;
    for (int i = 0; i < sampct; i++) {
        float *samples_l = samples + (i * numChans);
        float *samples_r = (samples + 1) + i * 2;
        // lap 1: left/mono channel
        float asamp = fabsf(*samples_l);
        float s2 = *samples_l * cc2;
        asamp = asamp * clipconst + 1.0f;
        *samples_l = s2 / asamp;
        if (numChans == 2) {
            // lap 2: right channel
            float asamp = fabsf(*samples_r);
            float s2 = *samples_r * cc2;
            asamp = asamp * clipconst + 1.0f;
            *samples_r = s2 / asamp;
        }
        samples_l = samples + (i * numChans);
        samples_r = (samples + 1) + i * 2;
    }
}

void DistortionEffect::SetParameters(DistortionEffect::Params const &params) {
    unk0 = params.drive * 0.01f;
}

void DistortionEffect::Reset() {}
