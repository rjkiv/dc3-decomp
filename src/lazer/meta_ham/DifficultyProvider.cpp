#include "meta_ham/DifficultyProvider.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "utl/Symbol.h"

DifficultyProvider::DifficultyProvider() : unk30(0) {}

Symbol DifficultyProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mDifficulties.size(), 0x76);
    return DifficultyToSym(mDifficulties[idx]);
}

bool DifficultyProvider::IsDifficultyUnlocked(Symbol s) const {
    if (TheGameMode->InMode("practice", true)) {
        return true;
    } else {
        // MetaPerformer call
    }
    return false;
}

void DifficultyProvider::InitData(RndDir *dir) {
    mDifficulties.clear();

    if (!TheGameMode->InMode("practice", true))
        mDifficulties.push_back(kDifficultyBeginner);

    mDifficulties.push_back(kDifficultyEasy);
    mDifficulties.push_back(kDifficultyMedium);
    mDifficulties.push_back(kDifficultyExpert);
}
