#pragma once
#include "obj/Data.h"
#include "utl/Symbol.h"
#include <vector>

enum MoveRating {
    kMoveRatingSuperPerfect = 0,
    kMoveRatingPerfect = 1,
    kMoveRatingAwesome = 2,
    kMoveRatingOk = 3,
    kNumMoveRatings = 4
};

MoveRating DetectFracToMoveRating(float, const std::vector<float> *);
float DetectFracToRatingFrac(float, const std::vector<float> *);
float RatingToDetectFrac(Symbol, const std::vector<float> *);
float RatingToRatingFrac(Symbol);
Symbol RatingState(int);
void RatingStateThreshold(int, Symbol &, float &, const std::vector<float> *);
int RatingStateToIndex(Symbol);
void ScoreUtlInit(const DataArray *);
