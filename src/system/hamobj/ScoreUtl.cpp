#include "hamobj/ScoreUtl.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

std::vector<Symbol> sRatingStates;
std::vector<float> sDefaultRatingThresholds;

MoveRating DetectFracToMoveRating(float detect_frac, const std::vector<float> *ratings) {
    if (!ratings)
        ratings = &sDefaultRatingThresholds;
    MILO_ASSERT(detect_frac >= 0 && detect_frac <= 1.0f, 0x22);
    for (int i = 0; i < ratings->size(); i++) {
        if (detect_frac >= (*ratings)[i])
            return (MoveRating)i;
    }
    return kNumMoveRatings;
}

float DetectFracToRatingFrac(float detect_frac, const std::vector<float> *thresholds) {
    MILO_ASSERT(detect_frac >= 0 && detect_frac <= 1.0f, 0x2F);
    if (!thresholds) {
        thresholds = &sDefaultRatingThresholds;
    }
    float f7 = 1;
    int size = thresholds->size();
    float threshSize = size - 1;
    float inv = 1 / threshSize;
    for (int i = 0; i < thresholds->size(); i++) {
        float cur = (*thresholds)[i];
        if (detect_frac >= cur) {
            if (i == 0) {
                return 1;
            } else {
                float f8;
                f8 = ((detect_frac - cur) / (f7 - cur)) + threshSize;
                f8 -= i;
                f8 *= inv;
                return f8;
            }
        }
        f7 = cur;
    }
    return 0;
}

float RatingToDetectFrac(Symbol rating, const std::vector<float> *thresholds) {
    if (!thresholds) {
        thresholds = &sDefaultRatingThresholds;
    }
    for (int i = 0; i < sRatingStates.size(); i++) {
        if (rating == sRatingStates[i]) {
            return (*thresholds)[i];
        }
    }
    MILO_NOTIFY("Could not find rating (%s)", rating);
    return 0;
}

float RatingToRatingFrac(Symbol rating) {
    int size = sRatingStates.size() - 1;
    for (int i = 0; i < sRatingStates.size(); i++) {
        if (rating == sRatingStates[i]) {
            return (float)(size - i) / (float)(size);
        }
    }
    MILO_NOTIFY("Could not find rating (%s)", rating);
    return 0;
}

void RatingStateThreshold(
    int index, Symbol &ratingState, float &thresh, const std::vector<float> *thresholds
) {
    if (!thresholds) {
        thresholds = &sDefaultRatingThresholds;
    }
    MILO_ASSERT_RANGE(index, 0, sRatingStates.size(), 0x77);
    MILO_ASSERT_RANGE(index, 0, thresholds->size(), 0x78);
    ratingState = sRatingStates[index];
    thresh = (*thresholds)[index];
}

int RatingStateToIndex(Symbol s) {
    for (int i = 0; i < sRatingStates.size(); i++) {
        if (s == sRatingStates[i]) {
            return i;
        }
    }
    MILO_NOTIFY("Could not find rating (%s)", s);
    return 0;
}

float GetScoreBonus(float detect_frac, const std::vector<float> *thresholds) {
    if (!thresholds) {
        thresholds = &sDefaultRatingThresholds;
    }
    MILO_ASSERT(detect_frac >= 0 && detect_frac <= 1.0f, 0x8D);
    float f9 = 0;
    float f11 = 0;
    for (int i = 0; i < thresholds->size(); i++) {
        if (detect_frac >= (*thresholds)[i]) {
            f9 = (*thresholds)[i];
            if (f11 <= (i - 1)) {
                f11 = (*thresholds)[i - 1];
            } else {
                f11 = 1;
            }
            break;
        }
    }
    return (detect_frac - f9) / (f11 - f9);
}

Symbol RatingState(int index) {
    MILO_ASSERT_RANGE(index, 0, sRatingStates.size(), 0xA7);
    return sRatingStates[index];
}

Symbol DetectFracToRating(
    float detect_frac, const std::vector<float> *ratings, int *move_rating
) {
    MoveRating mr = DetectFracToMoveRating(detect_frac, ratings);
    if (move_rating) {
        *move_rating = mr;
    }
    if (mr == kNumMoveRatings) {
        return gNullStr;
    } else {
        return sRatingStates[mr];
    }
}

void ScoreUtlInit(const DataArray *a) {
    sRatingStates.clear();
    sDefaultRatingThresholds.clear();

    DataArray *states = a->FindArray("feedback_states");
    DataArray *thresholds = a->FindArray("feedback_thresholds");
    MILO_ASSERT(states->Size() == thresholds->Size(), 0xB3);

    for (int i = 1; i < states->Size(); i++) {
        sRatingStates.push_back(states->Sym(i));
        sDefaultRatingThresholds.push_back(thresholds->Float(i));
    }

    MILO_ASSERT(sRatingStates.size() == kNumMoveRatings, 0xBA);
}
