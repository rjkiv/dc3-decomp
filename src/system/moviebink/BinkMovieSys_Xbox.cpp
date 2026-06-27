#include "bink.h"
#include "moviebink/BinkMovieSys.h"
#include "synth360/Synth.h"
#include "xdk/win_types.h"

void BinkMovieSys::PlatformInit() {
    if (TheXboxSynth) {
        // FIXME: param2 casted as an int, but expected 64-bit value
        BinkSetSoundSystem(BinkOpenXAudio2, (INT_PTR)TheXboxSynth->GetXAudio());
    }
}
