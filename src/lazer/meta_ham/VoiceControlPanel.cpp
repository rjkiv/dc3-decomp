#include "meta_ham/VoiceControlPanel.h"
#include "HamPanel.h"
#include "ProfileMgr.h"
#include "hamobj/Difficulty.h"
#include "os/PlatformMgr.h"
#include "utl/Symbol.h"

VoiceControlPanel::VoiceControlPanel()
    : unk44(0), unk48(false), unk4c(3.4028235e+38), unk50(0), unk54(false), unk55(false),
      unk56(false), unk64(false), unk68(0), unk6c(0) {}

VoiceControlPanel::~VoiceControlPanel() {}

void VoiceControlPanel::Poll() { HamPanel::Poll(); }

bool VoiceControlPanel::DifficultyLocked() const {
    if (unk58 == gNullStr)
        return false;
    else {
        static Symbol practice("practice");
        if (unk60 == practice)
            return false;
        else
            return !TheProfileMgr.IsDifficultyUnlocked(
                unk58, DifficultyToSym(mDifficulty)
            );
    }
}

bool VoiceControlPanel::ReadyToStart() const {
    return unk58 != gNullStr && mDifficulty != kNumDifficulties && unk60 != gNullStr
        && !DifficultyLocked();
}

void VoiceControlPanel::WakeUpScreenSaver() const {
    bool b = ThePlatformMgr.ScreenSaver();
    ThePlatformMgr.SetScreenSaver(false);
    ThePlatformMgr.SetScreenSaver(b);
}
