#include "meta_ham/DifficultyProvider.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "net/json-c/printbuf.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"
#include <cstdio>

DifficultyProvider::DifficultyProvider() : unk30(0) {}

Symbol DifficultyProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mDifficulties.size(), 0x76);
    return DifficultyToSym(mDifficulties[idx]);
}

bool DifficultyProvider::IsDifficultyUnlocked(Symbol s) const {
    if (TheGameMode->InMode("practice", true)) {
        return true;
    } else {
        MetaPerformer *performer = MetaPerformer::Current();
        MILO_ASSERT(performer, 0x6a);
        return performer->IsDifficultyUnlocked(s);
    }
}

void DifficultyProvider::InitData(RndDir *dir) {
    mDifficulties.clear();

    if (!TheGameMode->InMode("practice", true))
        mDifficulties.push_back(kDifficultyBeginner);

    mDifficulties.push_back(kDifficultyEasy);
    mDifficulties.push_back(kDifficultyMedium);
    mDifficulties.push_back(kDifficultyExpert);
}

void DifficultyProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mDifficulties.size(), 0x3c);
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x3f);
    Symbol dataSymbol = DataSymbol(data);
    if (uiListLabel->Matches("player_present")) {
        static Symbol player_present("player_present");
        HamPlayerData *pPlayerData = TheGameData->Player(unk30);
        PropertyEventProvider *pep = pPlayerData->Provider();
        const DataNode *node = pep->Property(player_present, true);
        // something
    } else if (uiListLabel->Matches("lock")) {
        if (!IsDifficultyUnlocked(dataSymbol)) {
            uiLabel->SetIcon('B');
            return;
        }
        app_label->SetTextToken(gNullStr);
    }
}
