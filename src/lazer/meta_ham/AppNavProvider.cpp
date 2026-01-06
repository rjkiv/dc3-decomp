#include "AppNavProvider.h"
#include "HamStarsDisplay.h"
#include "hamobj/Difficulty.h"
#include "ui/UIListCustom.h"
#include "AppLabel.h"

BEGIN_CUSTOM_HANDLERS(AppNavProvider)
    HANDLE_SUPERCLASS(HamNavProvider)
END_CUSTOM_HANDLERS

AppNavProvider::~AppNavProvider() {}

void AppNavProvider::Text(int i1, int data, UIListLabel *slot, UILabel *label) const {
    AppLabel *applabel = dynamic_cast<AppLabel *>(label);
    if (slot->Matches("practice_diff")) {
        const NavItem &cur = mNavItems[data];
        if (cur.unkc == 2) {
            applabel->SetBestPracticeDifficulty(cur.mSongID);
        }
    } else if (slot->Matches("practice_score")) {
        const NavItem &cur = mNavItems[data];
        if (cur.unkc == 2) {
            applabel->SetPracticeScore(cur.mSongID, kNumDifficulties);
        }
    } else if (slot->Matches("song") && mNavItems[data].mLabels.size() == 2) {
        applabel->SetPrelocalizedString(String(mNavItems[data].mLabels[0]));
    } else if (slot->Matches("song_length") && mNavItems[data].mLabels.size() == 2) {
        applabel->SetPrelocalizedString(String(mNavItems[data].mLabels[1]));
    } else
        HamNavProvider::Text(i1, data, slot, label);
}

void AppNavProvider::Custom(
    int i1, int i2, UIListCustom *listcustom, Hmx::Object *obj
) const {
    if (listcustom->Matches("stars")) {
        HamStarsDisplay *hsd = dynamic_cast<HamStarsDisplay *>(obj);
        if (mNavItems[i2].unkc == 1) {
            hsd->SetSong(mNavItems[i2].mSongID);
        }
    }
}
