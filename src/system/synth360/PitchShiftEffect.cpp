#include "synth360/PitchShiftEffect.h"
#include "synth360/soundtouch/SoundTouch.h"
#include <cstring>

PitchShiftEffect::PitchShiftEffect() : mLastPitch(1), mLastChannels(2) {
    mSoundTouch = new soundtouch::SoundTouch();
    mSoundTouch->setSampleRate(48000);
    mSoundTouch->setChannels(2);
    mSoundTouch->setSetting(0, 1);
}

PitchShiftEffect::~PitchShiftEffect() { RELEASE(mSoundTouch); }

void PitchShiftEffect::DoProcess(
    const PitchShiftEffectParams &params, float *f, unsigned int, unsigned int channels
) {
    if (mLastChannels != (int)channels) {
        mSoundTouch->setChannels(channels);

        mLastChannels = channels;
    }
    if (params.mParamPitchRatio != mLastPitch && 0 < params.mParamPitchRatio) {
        mSoundTouch->setPitch(params.mParamPitchRatio);
        mSoundTouch->flush();
        mSoundTouch->clear();
        mStarted = 0;
        mLastPitch = params.mParamPitchRatio;
    }

    if (mLastPitch != 1.0f) {
        mSoundTouch->putSamples(f, 0x100);
        if (!mStarted && 0x400 <= mSoundTouch->numSamples()) {
            mStarted = true;
        }
        unsigned int size;
        if (mStarted) {
            unsigned int receiveSamples = mSoundTouch->receiveSamples(f, 0x100);
            if (0x100 <= receiveSamples) {
                return;
            }
            mStarted = false;
            size = (0x100 - receiveSamples) * channels * 4;
            f = f + receiveSamples * channels;
            memset(f, 0, size);
        } else {
            memset(f, 0, channels << 10);
        }
    }
}
