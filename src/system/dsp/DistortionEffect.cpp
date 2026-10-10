#include "dsp/DistortionEffect.h"
#include "math/Utl.h"
#include "os/Debug.h"

DistortionEffect::DistortionEffect(IXAudioBatchAllocator *) : mDrive(0) {}

void DistortionEffect::Process(float *samples, int sampct, int numChans) {
    MILO_ASSERT(numChans <= 2, 27);
    float k = mDrive / std::max(1.0f - mDrive, 0.01f) * 2.0f;
    for (int i = 0; i < sampct; i++) {
        float &l = samples[i * numChans];
        l = l * (k + 1.0f) / (fabsf(l) * k + 1.0f);
        if (numChans == 2) {
            float &r = samples[i * numChans + 1];
            r = r * (k + 1.0f) / (fabsf(r) * k + 1.0f);
        }
    }
}

void DistortionEffect::SetParameters(DistortionEffect::Params const &params) {
    mDrive = params.drive * 0.01f;
}

void DistortionEffect::Reset() {}
