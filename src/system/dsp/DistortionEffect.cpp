#include "dsp/DistortionEffect.h"
#include "os/Debug.h"

DistortionEffect::DistortionEffect(IXAudioBatchAllocator *) : unk0(0) {}

void DistortionEffect::Process(float *f, int i1, int numChans) {
    MILO_ASSERT(numChans <= 2, 0x1b);
}

void DistortionEffect::SetParameters(DistortionEffect::Params const &params) {
    unk0 = params.drive * 0.01f;
}

void DistortionEffect::Reset() {}
