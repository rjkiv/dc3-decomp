#include "bink.h"
#include "moviebink/BinkMovieSys.h"
#include "synth360/Synth.h"
#include "xdk/win_types.h"

void BinkMovieSys::PlatformInit() {
    if (TheXboxSynth) {
        BinkSetSoundSystem(BinkOpenXAudio2, (INT_PTR)TheXboxSynth->GetXAudio());
    }
}

void BinkMovieSys::PlatformStoreCache(void *buf, uint len) {
    if (len == 0)
        return;
    u32 i = (len - 1) / 0x80 + 1;
    do {
        __dcbst(0, buf);
        buf = (char *)buf + 0x80;
    } while (--i);
}
