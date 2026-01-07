#include "meta_ham/PracticeChoosePanel.h"
#include "HamPanel.h"
#include "HamProfile.h"
#include "ProfileMgr.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDir.h"
#include "hamobj/PracticeSection.h"
#include "hamobj/ScoreUtl.h"
#include "math/Utl.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/MoveRatingHistory.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "rndobj/Text.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "ui/UIPanel.h"
#include "utl/Locale.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

static int sIndex;

#pragma region StepMoves

bool StepMoves::IsRecap() const {
    static Symbol review("review");
    return mType == review;
}

float StepMoves::GetOverallRating() const {
    float overallRating = 0;
    for (int i = 0; i < kNumMoveRatings; i++) {
        overallRating += GetRatingHistory(i);
    }
    return overallRating;
}

int StepMoves::GetRatingHistory(int i) const {
    int ratingHistory = -1;
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (mMoves.size() != 0 && pProfile) {
        MoveRatingHistory *mrh = pProfile->GetMoveRatingHistory();
        ratingHistory = mrh->GetRating(mMoves.front()->DisplayName(), i);
    }
    return ratingHistory;
}

String StepMoves::GetDisplayName(bool b1) const {
    int numMoves = mMoves.size();
    String out(gNullStr);
    if (IsRecap()) {
        static Symbol recap_name("recap_name");
        out = Localize(recap_name, nullptr, TheLocale);
    } else if (!mNameOverride.empty()) {
        out = mNameOverride;
    } else if (numMoves == 1) {
        out = mMoves[0]->DisplayName();
    } else if (numMoves <= 1) {
        // do nothing
    } else if (numMoves == 2
               && stricmp(mMoves[0]->DisplayName(), mMoves[1]->DisplayName()) == 0) {
        out = mMoves[0]->DisplayName();
    } else {
        static Symbol sequence("sequence");
        out = MakeString(
            "%s %s", mMoves[0]->DisplayName(), Localize(sequence, nullptr, TheLocale)
        );
    }
    if (unk28) {
        if (!out.empty() && !b1) {
            out += MakeString(" %d", unk28);
        }
    }
    return out;
}

BEGIN_CUSTOM_PROPSYNC(StepMoves)
    SYNC_PROP(moves, o.mMoves)
    SYNC_PROP(selected, o.mSelected)
END_CUSTOM_PROPSYNC

#pragma endregion
#pragma region PracticeChoosePanel

PracticeChoosePanel::PracticeChoosePanel() {}

BEGIN_HANDLERS(PracticeChoosePanel)
    HANDLE_EXPR(num_selected, NumSelected())
    HANDLE_ACTION(set_up_custom_section, SetUpCustomSection())
    HANDLE_ACTION(toggle_sort, ToggleSort())
    HANDLE_EXPR(want_to_auto_select_recommended, WantToAutoSelectRecommended())
    HANDLE_EXPR(step_start_beat, StepStartBeat(_msg->Int(2)))
    HANDLE_EXPR(step_end_beat, StepEndBeat(_msg->Int(2)))
    HANDLE_EXPR(step_is_recap, mStepMoves[_msg->Int(2)].IsRecap())
    HANDLE_ACTION(clear_memory, ClearAndShrink(unk54))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

BEGIN_PROPSYNCS(PracticeChoosePanel)
    SYNC_PROP(step_moves, mStepMoves)
END_PROPSYNCS

void PracticeChoosePanel::Exit() {
    UIPanel::Exit();
    DataVariable("auto_select_practice_moves") = 0;
    unk4c = TheGameData->GetSong();
    unk50 = DifficultyToSym(TheGameData
                                ->Player(TheHamProvider->Property("ui_nav_player")->Int())
                                ->GetDifficulty());
    unk54.clear();
    FOREACH (it, mStepMoves) {
        if (it->mSelected) {
            unk54.push_back(it->unk2c);
        }
    }
}

void PracticeChoosePanel::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    static Symbol gameplay("gameplay");
    static Symbol recap_numbered("recap_numbered");
    MILO_ASSERT_RANGE(data, 0, mStepMoves.size(), 0xB4);
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0xB7);
    if (slot->Matches("step_name")) {
        if (mStepMoves[data].MoveRecapCheck()) {
            app_label->SetCapsMode(RndText::kForceUpper);
            app_label->SetTextToken(gameplay);
        } else {
            app_label->SetCapsMode(RndText::kCapsModeNone);
            app_label->SetStepMoveName(mStepMoves[data]);
        }
    }
    if (slot->Matches("checkbox")) {
        if (mStepMoves[data].MoveRecapCheck()) {
            app_label->SetIcon('\0');
        } else {
            app_label->SetChecked(mStepMoves[data].mSelected);
        }
    }
}

RndMat *PracticeChoosePanel::Mat(int, int idx, UIListMesh *slot) const {
    if (slot->Matches("problem_callout")) {
        if (WantToAutoSelectRecommended()
            && CheckIfAutoSelected(mStepMoves[idx].mMoves)) {
            return unk60;
        } else {
            int i4 = 0;
            int i5 = 0;
            for (int i = 0; i < 4; i++) {
                int history = mStepMoves[idx].GetRatingHistory(i);
                if (history >= 2) {
                    i4++;
                }
                if (history >= 0) {
                    i5++;
                }
            }
            if (mStepMoves[idx].HistoryCheck()) {
                return nullptr;
            }
            if (i5 <= 0) {
                return nullptr;
            }
            if ((float)i4 / (float)i5 <= 0.49f) {
                return nullptr;
            }
            return unk60;
        }
    } else {
        return nullptr;
    }
}

Symbol PracticeChoosePanel::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mStepMoves.size(), 0xFF);
    static Symbol gameplay("gameplay");
    if (mStepMoves[idx].MoveRecapCheck()) {
        return gameplay;
    } else {
        return mStepMoves[idx].mStart;
    }
}

int PracticeChoosePanel::DataIndex(Symbol s) const {
    for (int i = 0; i < mStepMoves.size(); i++) {
        if (mStepMoves[i].mStart == s) {
            return i;
        }
    }
    return -1;
}

int PracticeChoosePanel::NumData() const { return mStepMoves.size(); }

void PracticeChoosePanel::InitData(RndDir *dir) {
    TheGame->GetMaster()->SetMaps();
    if (unk4c != TheGameData->GetSong()
        || unk50
            != DifficultyToSym(
                TheGameData->Player(TheHamProvider->Property("ui_nav_player")->Int())
                    ->GetDifficulty()
            )) {
        ClearAndShrink(unk54);
    }
    mStepMoves.clear();
    StepMoves stepMoves;
    mStepMoves.push_back(stepMoves);
    MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
    MILO_ASSERT(moves, 0x47);
    PracticeSection *section = nullptr;
    for (ObjDirItr<PracticeSection> it(moves, true); it != nullptr; ++it) {
        Difficulty diff = it->GetDifficulty();
        section = it;
        if (diff
            == TheGameData->Player(TheHamProvider->Property("ui_nav_player")->Int())
                   ->GetDifficulty()) {
            break;
        }
    }
    MILO_ASSERT(section, 0x51);
    static Symbol learn("learn");
    static Symbol review("review");
    int stepNum = 1;
    auto &steps = section->Steps();
    FOREACH (it, steps) {
        if (it->mType == learn || it->mType == review) {
            StepMoves innerSteps;
            (PracticeStep &)(innerSteps) = *it;
            innerSteps.mMoves = GetMovesInStep(*it);
            innerSteps.unk2c = stepNum;
            if (WantToAutoSelectRecommended()) {
                innerSteps.mSelected = CheckIfAutoSelected(innerSteps.mMoves);
            } else {
                FOREACH (moveIt, unk54) {
                    if (*moveIt == stepNum) {
                        innerSteps.mSelected = true;
                        break;
                    }
                }
            }
            mStepMoves.push_back(innerSteps);
            stepNum++;
        }
    }
    mStepMoves.push_back(stepMoves);
    FOREACH (it, mStepMoves) {
        it->unk28 = GetStepNumber(*it);
    }
    unk60 = dir->Find<RndMat>("problem_callout.mat");
    sIndex = 0;
}

int PracticeChoosePanel::NumSelected() const {
    int num = 0;
    FOREACH (it, mStepMoves) {
        if (it->mSelected) {
            num++;
        }
    }
    return num;
}

void PracticeChoosePanel::ToggleSort() {
    sIndex = (sIndex + 1) % 2;
    std::sort(mStepMoves.begin(), mStepMoves.end());
}

bool PracticeChoosePanel::WantToAutoSelectRecommended() const {
    return DataVariable("auto_select_practice_moves").Int();
}

float PracticeChoosePanel::StepStartBeat(int i1) const {
    const StepMoves &cur = mStepMoves[i1];
    if (cur.MoveRecapCheck()) {
        return -1;
    } else {
        return Max(0.0f, TheHamDirector->BeatFromTag(cur.mStart));
    }
}

float PracticeChoosePanel::StepEndBeat(int i1) const {
    const StepMoves &cur = mStepMoves[i1];
    if (cur.MoveRecapCheck()) {
        return -1;
    } else {
        return Max(0.0f, TheHamDirector->BeatFromTag(cur.mEnd));
    }
}

bool PracticeChoosePanel::CheckIfAutoSelected(std::vector<HamMove *> const &moves) const {
    if (MetaPerformer::Current()->IsRecommendedPracticeMoveGroup(moves)) {
        return true;
    } else {
        for (int i = 0; i < moves.size(); i++) {
            if (MetaPerformer::Current()->IsRecommendedPracticeMove(
                    moves[i]->DisplayName()
                )) {
                return true;
            }
        }
        return false;
    }
}

int PracticeChoosePanel::GetStepNumber(const StepMoves &moves) const {
    int i5 = 0;
    int i4 = 0;
    bool b1 = false;
    FOREACH (it, mStepMoves) {
        if (it->GetDisplayName(true) == moves.GetDisplayName(true)) {
            if (it == &moves) {
                b1 = true;
            }
            if (!b1) {
                i4++;
            }
            i5++;
        }
    }
    if (i5 <= 1) {
        return 0;
    } else {
        return i4 + 1;
    }
}

void PracticeChoosePanel::SetUpCustomSection() {
    int i4 = 0;
    PracticeSection *customSect =
        TheHamDirector->GetWorld()->Find<PracticeSection>("custom_sect");
    customSect->ClearSteps();
    FOREACH (it, mStepMoves) {
        StepMoves cur = *it;
        PracticeStep step(cur);
        if (cur.mSelected) {
            if (cur.IsRecap()) {
                i4 = 0;
            } else {
                int i1 = cur.mMoves.size();
                i4 += i1;
                if (i4 >= 12) {
                    step.mBoundary = true;
                    i4 = i1;
                }
            }
            customSect->AddStep(step);
        }
    }
    static Symbol recap_results("recap_results");
    PracticeStep step;
    step.mType = recap_results;
    customSect->AddStep(step);
}

std::vector<HamMove *> PracticeChoosePanel::GetMovesInStep(PracticeStep step) {
    MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
    std::vector<HamMove *> out;
    static Symbol learn("learn");
    if (step.mType == learn) {
        int start = Round(TheHamDirector->BeatFromTag(step.mStart));
        int end = Round(TheHamDirector->BeatFromTag(step.mEnd));
        int diff = (end - start) / 4;
        if (diff <= 0) {
            MILO_NOTIFY(
                "numMoves = %d between start tag '%s' and end tag '%s'",
                diff,
                step.mStart,
                step.mEnd
            );
        }
        for (int i = 0; i < diff; i++) {
            Symbol name = TheHamDirector->MoveNameFromBeat(i * 4, 0);
            out.push_back(moves->Find<HamMove>(name.Str()));
        }
    }
    return out;
}

#pragma endregion
