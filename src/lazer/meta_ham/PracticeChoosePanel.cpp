#include "meta_ham/PracticeChoosePanel.h"
#include "HamPanel.h"
#include "HamProfile.h"
#include "ProfileMgr.h"
#include "hamobj/PracticeSection.h"
#include "meta_ham/MoveRatingHistory.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

#pragma region StepMoves

StepMoves::StepMoves() : unk24(false), unk28(), unk2c() {}

StepMoves::~StepMoves() {}

bool StepMoves::IsRecap() const {
    static Symbol review("review");
    return mType == review;
}

float StepMoves::GetOverallRating() const {
    float overallRating = 0;
    for (int i = 0; i < 4; i++) {
        overallRating += GetRatingHistory(i);
    }
    return overallRating;
}

int StepMoves::GetRatingHistory(int i) const {
    int ratingHistory = -1;
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (unk18.size() != 0 && pProfile) {
        MoveRatingHistory *mrh = pProfile->GetMoveRatingHistory();
        ratingHistory = mrh->GetRating(unk18.front()->DisplayName(), i);
    }
    return ratingHistory;
}

#pragma endregion StepMoves
#pragma region PracticeChoosePanel

PracticeChoosePanel::PracticeChoosePanel() {}

BEGIN_PROPSYNCS(PracticeChoosePanel)
    SYNC_PROP(step_moves, unk4c) // should be 0x40
END_PROPSYNCS

void PracticeChoosePanel::Enter() { HamPanel::Enter(); }

int PracticeChoosePanel::NumData() const { return unk40.size(); }

bool PracticeChoosePanel::WantToAutoSelectRecommended() const {
    DataNode &autoSelPracMoves = DataVariable("auto_select_practice_moves");
    return autoSelPracMoves.Int() != 0;
}

#pragma endregion PracticeChoosePanel
