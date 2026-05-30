#include "synth_xbox/PitchShiftEffect.h"
#include "synth_xbox/soundtouch/source/SoundTouch/SoundTouch.h"

PitchShiftEffect::PitchShiftEffect() : unk68(1), unk6c(2) {
    mSoundTouch = new soundtouch::SoundTouch();
    mSoundTouch->setSampleRate(48000);
    mSoundTouch->setChannels(2);
    mSoundTouch->setSetting(0, 1);
}

PitchShiftEffect::~PitchShiftEffect() { RELEASE(mSoundTouch); }
