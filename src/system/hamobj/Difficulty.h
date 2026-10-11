#pragma once
#include "utl/Symbol.h"

enum Difficulty {
    /** "Easy" Difficulty */
    kDifficultyEasy = 0,
    /** "Medium" Difficulty */
    kDifficultyMedium = 1,
    /** "Hard" Difficulty */
    kDifficultyExpert = 2,
    kNumDifficultiesDC2 = 3,
    /** "Beginner" Difficulty */
    kDifficultyBeginner = 3,
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

/** Beginner was added only in DC3, this would suggest songs from older games just use
 * Easy in it's place. Whether it is true for stock songs (beginner mode added via song
 * updates) must be yet verified. */
Difficulty LegacyDifficulty(Difficulty d) {
    return d == kDifficultyBeginner ? kDifficultyEasy : d;
}
