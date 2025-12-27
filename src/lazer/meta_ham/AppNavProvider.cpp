#include "AppNavProvider.h"
#include "HamStarsDisplay.h"
#include "ui/UIListCustom.h"
#include "AppLabel.h"

BEGIN_CUSTOM_HANDLERS(AppNavProvider)
HANDLE_SUPERCLASS(HamNavProvider)
END_CUSTOM_HANDLERS

void AppNavProvider::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    AppLabel *applabel = dynamic_cast<AppLabel *>(label);
    if (listlabel->Matches("practice_diff")) {
        if (mNavItems[i2].unkc != 2) {
            return;
        }
        applabel->SetBestPracticeDifficulty(mNavItems[i2].unk8);
        return;
    }
    if (listlabel->Matches("practice_score")) {
        if (mNavItems[i2].unkc != 2) {
            return;
        }
        applabel->SetPracticeScore(mNavItems[i2].unkc, kNumDifficulties);
        return;
    }

    if (listlabel->Matches("song")) {
        if (mNavItems.size() == 8) {
            label->SetPrelocalizedString(String(mNavItems[i2].mLabels[0]));

        }
    }
    else if (listlabel->Matches("song_length")) {
        if (mNavItems.size() != 8) {
            label->SetPrelocalizedString(String(mNavItems[i2].mLabels[1]));
        }
    }
    else
        HamNavProvider::Text(i1, i2, listlabel, label);
}

void AppNavProvider::Custom(int i1, int i2, UIListCustom *listcustom, Hmx::Object *obj) const {
    if (listcustom->Matches("stars")) {
        HamStarsDisplay *hsd = dynamic_cast<HamStarsDisplay *>(obj);
        if (mNavItems[i2].unkc == 1) {
            hsd->SetSong(mNavItems[i2].unk8);
        }

    }
}