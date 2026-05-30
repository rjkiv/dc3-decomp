#include "synth360/PitchShiftEffect.h"
#include "synth360/soundtouch/SoundTouch.h"

PitchShiftEffect::PitchShiftEffect() : unk68(1), unk6c(2) {
    mSoundTouch = new soundtouch::SoundTouch();
    mSoundTouch->setSampleRate(48000);
    mSoundTouch->setChannels(2);
    mSoundTouch->setSetting(0, 1);
}

PitchShiftEffect::~PitchShiftEffect() { RELEASE(mSoundTouch); }
