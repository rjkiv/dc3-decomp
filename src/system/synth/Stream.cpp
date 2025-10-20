#include "synth/Stream.h"
#include "Synth.h"
#include "synth/Faders.h"

Stream::Stream() {
    static Symbol _default("_default");
    mFaders = new FaderGroup(nullptr);
    mFaders->AddLocal(_default)->SetVolume(0);
    mFaders->Add(TheSynth->MasterFader());
}

void Stream::SetVolume(float vol) {
    static Symbol _default("_default");
    mFaders->FindLocal(_default, true)->SetVolume(vol);
}
