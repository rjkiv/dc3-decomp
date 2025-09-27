#pragma once
#include "utl/Symbol.h"

enum Difficulty {
    kDifficultyEasy = 0,
    kDifficultyMedium = 1,
    kDifficultyExpert = 2,
    kDifficultyBeginner = 3,
    kNumDifficultiesDC2 = 3,
    kNumDifficulties = 4
};

Symbol DifficultyToSym(Difficulty);
Difficulty SymToDifficulty(Symbol);
bool IsEasierDifficulty(Difficulty, Difficulty);
bool IsHarderDifficulty(Difficulty, Difficulty);
Difficulty DifficultyOneHarder(Difficulty);
Difficulty DifficultyOneEasier(Difficulty);
void DifficultyInit();
Difficulty DefaultDifficulty();
Difficulty EasiestDifficulty();

Difficulty LegacyDifficulty(Difficulty d) {
    return d == kDifficultyBeginner ? kDifficultyEasy : d;
}
