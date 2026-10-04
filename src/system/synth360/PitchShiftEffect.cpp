#include "synth360/PitchShiftEffect.h"
#include "synth360/soundtouch/SoundTouch.h"
#include <cstring>

PitchShiftEffect::PitchShiftEffect() : unk68(1), unk6c(2) {
    mSoundTouch = new soundtouch::SoundTouch();
    mSoundTouch->setSampleRate(48000);
    mSoundTouch->setChannels(2);
    mSoundTouch->setSetting(0, 1);
}

PitchShiftEffect::~PitchShiftEffect() { RELEASE(mSoundTouch); }

void PitchShiftEffect::DoProcess(
    const PitchShiftEffectParams &params, float *f, unsigned int, unsigned int channels
) {
    if (unk6c != (int)channels) {
        mSoundTouch->setChannels(channels);

        unk6c = channels;
    }
    if (params.unk0 != unk68 && 0 < params.unk0) {
        mSoundTouch->setPitch(params.unk0);
        mSoundTouch->flush();
        mSoundTouch->clear();
        unk64 = 0;
        unk68 = params.unk0;
    }

    if (unk68 != 1.0f) {
        mSoundTouch->putSamples(f, 0x100);
        if (!unk64 && 0x400 <= mSoundTouch->numSamples()) {
            unk64 = true;
        }
        unsigned int size;
        if (unk64) {
            unsigned int receiveSamples = mSoundTouch->receiveSamples(f, 0x100);
            if (0x100 <= receiveSamples) {
                return;
            }
            unk64 = false;
            size = (0x100 - receiveSamples) * channels * 4;
            f = f + receiveSamples * channels;
            memset(f, 0, size);
        } else {
            memset(f, 0, channels << 10);
        }
    }
}
