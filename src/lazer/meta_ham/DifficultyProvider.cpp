#include "meta_ham/DifficultyProvider.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"
#include <cstdio>

DifficultyProvider::DifficultyProvider() : mPlayerIndex(0) {}

void DifficultyProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mDifficulties.size(), 0x3c);
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x3f);
    Symbol diffSym = DataSymbol(data);
    if (uiListLabel->Matches("difficulty")) {
        static Symbol player_present("player_present");
        int otherPlayer = !mPlayerIndex;
        if (TheGameData->Player(mPlayerIndex)->Provider()->Property(player_present)->Int()
                != 1
            && TheGameData->Player(otherPlayer)->Provider()->Property(player_present)->Int()
                != 0) {
            char buffer[32];
            buffer[0] = 0;
            memset(&buffer[1], 0, 31);
            sprintf(buffer, "%s%s", diffSym.Str(), "_title");
            uiLabel->SetTextToken(buffer);
        } else {
            uiLabel->SetTextToken(diffSym);
        }
    } else if (uiListLabel->Matches("lock")) {
        if (!IsDifficultyUnlocked(diffSym)) {
            app_label->SetIcon('B');
        } else {
            app_label->SetTextToken(gNullStr);
        }
    }
}

Symbol DifficultyProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mDifficulties.size(), 0x76);
    return DifficultyToSym(mDifficulties[idx]);
}

void DifficultyProvider::InitData(RndDir *dir) {
    mDifficulties.clear();
    if (!TheGameMode->InMode("practice", true)) {
        mDifficulties.push_back(kDifficultyBeginner);
    }
    mDifficulties.push_back(kDifficultyEasy);
    mDifficulties.push_back(kDifficultyMedium);
    mDifficulties.push_back(kDifficultyExpert);
}

bool DifficultyProvider::IsDifficultyUnlocked(Symbol diffSym) const {
    if (TheGameMode->InMode("practice", true)) {
        return true;
    } else {
        MetaPerformer *performer = MetaPerformer::Current();
        MILO_ASSERT(performer, 0x6a);
        return performer->IsDifficultyUnlocked(diffSym);
    }
}
