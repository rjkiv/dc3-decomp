#include "hamobj/Difficulty.h"
#include "Difficulty.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "os/Debug.h"

Difficulty DefaultDifficulty() { return kDifficultyEasy; }

Symbol DifficultyToSym(Difficulty d) {
    MILO_ASSERT((0) <= (d) && (d) < (kNumDifficulties), 0x28);
    return DataGetMacro("DIFF_SYMBOLS")->Sym(d);
}

int difficulty_hardness(Difficulty d) {
    MILO_ASSERT((0) <= (d) && (d) < (kNumDifficulties), 0x2F);
    switch (d) {
    case kDifficultyEasy:
        return 1;
    case kDifficultyMedium:
        return 2;
    case kDifficultyExpert:
        return 3;
    case kDifficultyBeginner:
        return 0;
    default:
        return 4;
    }
}

bool IsEasierDifficulty(Difficulty d1, Difficulty d2) {
    return difficulty_hardness(d1) < difficulty_hardness(d2);
}

bool IsHarderDifficulty(Difficulty d1, Difficulty d2) {
    return difficulty_hardness(d1) > difficulty_hardness(d2);
}

Difficulty DifficultyOneHarder(Difficulty d) {
    MILO_ASSERT((0) <= (d) && (d) < (kNumDifficulties), 0x4B);
    switch (d) {
    case kDifficultyBeginner:
        return kDifficultyEasy;
    case kDifficultyEasy:
        return kDifficultyMedium;
    case kDifficultyMedium:
        return kDifficultyExpert;
    case kDifficultyExpert:
        return kNumDifficulties;
    default:
        return kNumDifficulties;
    }
}

Difficulty DifficultyOneEasier(Difficulty d) {
    MILO_ASSERT((0) <= (d) && (d) < (kNumDifficulties), 0x5E);
    switch (d) {
    case kDifficultyBeginner:
        return kNumDifficulties;
    case kDifficultyEasy:
        return kDifficultyBeginner;
    case kDifficultyMedium:
        return kDifficultyEasy;
    case kDifficultyExpert:
        return kDifficultyMedium;
    default:
        return kNumDifficulties;
    }
}

Difficulty SymToDifficulty(Symbol s) {
    for (int i = 0; i < kNumDifficulties; i++) {
        if (s == DifficultyToSym((Difficulty)i)) {
            return (Difficulty)i;
        }
    }
    MILO_ASSERT(false, 0x13);
    return kNumDifficulties;
}

DataNode OnDifficultyToSym(DataArray *a) {
    return DifficultyToSym((Difficulty)a->Int(1));
}

DataNode OnSymToDifficulty(DataArray *a) { return SymToDifficulty(a->Sym(1)); }

DataNode OnDifficultyOneEasier(DataArray *a) {
    return DifficultyOneEasier((Difficulty)a->Int(1));
}

DataNode OnDifficultyOneHarder(DataArray *a) {
    return DifficultyOneHarder((Difficulty)a->Int(1));
}

DataNode OnIsEasierDifficulty(DataArray *a) {
    return IsEasierDifficulty((Difficulty)a->Int(1), (Difficulty)a->Int(2));
}

DataNode OnIsHarderDifficulty(DataArray *a) {
    return IsHarderDifficulty((Difficulty)a->Int(1), (Difficulty)a->Int(2));
}

void DifficultyInit() {
    DataRegisterFunc("difficulty_to_sym", OnDifficultyToSym);
    DataRegisterFunc("sym_to_difficulty", OnSymToDifficulty);
    DataRegisterFunc("difficulty_one_easier", OnDifficultyOneEasier);
    DataRegisterFunc("difficulty_one_harder", OnDifficultyOneHarder);
    DataRegisterFunc("is_easier_difficulty", OnIsEasierDifficulty);
    DataRegisterFunc("is_harder_difficulty", OnIsHarderDifficulty);
}
