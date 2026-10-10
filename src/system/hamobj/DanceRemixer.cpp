#include "hamobj/DanceRemixer.h"
#include "MoveMgr.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDetector.h"
#include "hamobj/MoveDir.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"

DanceRemixer::DanceRemixer() {}
DanceRemixer::~DanceRemixer() { HandleType(Message("deinit")); }

Symbol OnMoveVariantFromHamMove(const DataArray *array) {
    MILO_ASSERT(array->Size() == 3, 0x21C);
    DanceRemixer *remixer = array->Obj<DanceRemixer>(0);
    HamMove *move = array->Obj<HamMove>(2);
    MILO_ASSERT(move, 0x21F);
    const MoveVariant *mv = remixer->MoveVariantFromHamMove(move);
    MILO_ASSERT(mv, 0x221);
    return mv ? mv->GetName() : "";
}

BEGIN_HANDLERS(DanceRemixer)
    HANDLE_ACTION(reset, Reset())
    HANDLE_ACTION(post_move_finished, PostMoveFinished())
    HANDLE_ACTION(set_jump, SetJump(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(clear_jump, ClearJump())
    HANDLE_EXPR(jump_from_beat, mJumpFromIdx * 4)
    HANDLE_EXPR(jump_to_beat, mJumpToIdx * 4)
    HANDLE_EXPR(jump_from_measure, mJumpFromIdx + 1)
    HANDLE_EXPR(jump_to_measure, mJumpToIdx + 1)
    HANDLE_EXPR(jumped_beat, JumpedBeat(_msg->Float(2)))
    HANDLE_EXPR(jumped_measure, JumpedMoveIdx(_msg->Int(2) - 1) + 1)
    HANDLE_EXPR(jumped_measure_add, JumpedMeasureAdd(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(
        jumped_measure_steps_between,
        JumpedMeasureStepsBetween(_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_EXPR(scored_measure, ScoredDanceMeasure(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(set_unscored_measure, SetUnscoredMeasure(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(
        set_unscored_measure_range,
        SetUnscoredMeasureRange(_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_ACTION(clear_unscored_measure, ClearUnscoredMeasure(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(
        clear_unscored_measure_range,
        ClearUnscoredMeasureRange(_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_ACTION(clear_unscored_measures, mUnscoredMeasures[_msg->Int(2)].clear())
    HANDLE_EXPR(move_variant_from_ham_move, OnMoveVariantFromHamMove(_msg))
    HANDLE_EXPR(measures_total, mSongMeasures)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void DanceRemixer::SetUnscoredMeasure(int x, int y) { mUnscoredMeasures[x].insert(y); }
void DanceRemixer::ClearUnscoredMeasure(int x, int y) { mUnscoredMeasures[x].erase(y); }

BEGIN_PROPSYNCS(DanceRemixer)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(DanceRemixer)
    SAVE_SUPERCLASS(Hmx::Object)
END_SAVES

BEGIN_COPYS(DanceRemixer)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(DanceRemixer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSongMeasures)
        for (int i = 0; i < 2; i++) {
            COPY_MEMBER(mUnscoredMeasures[i])
        }
        COPY_MEMBER(mVariantsNeededInMemory)
        COPY_MEMBER(mReloadVariantsNeededInMemory)
        COPY_MEMBER(mJumpFromIdx)
        COPY_MEMBER(mJumpToIdx)
        COPY_MEMBER(mJumpMap)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(DanceRemixer)
    Hmx::Object::Load(bs);
END_LOADS

void DanceRemixer::Init(int aNumMeasures) {
    if (TheMoveMgr->mWholeMoveGraph.mNodes.size() == 0) {
        MILO_FAIL("Failed to load move graph for: %s\n", TheGameData->GetSong());
    }
    mSongMeasures = aNumMeasures;
    for (int i = 0; i < 2; i++) {
        TheMoveMgr->mRoutineParents[i].resize(mSongMeasures);
        TheMoveMgr->mRoutinePreferredVariants[i].resize(mSongMeasures);
        TheMoveMgr->mRoutine[i].resize(mSongMeasures);
    }
    ClearJump();
    HandleType(Message("post_init"));
}

void DanceRemixer::Reset() {
    mVariantsNeededInMemory.clear();
    mReloadVariantsNeededInMemory = false;
    ClearJump();
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < mSongMeasures; i++) {
        }
        mUnscoredMeasures[i].clear();
    }
    HandleType(Message("post_reset"));
}

DataNode DanceRemixer::OnMovePassed(DataArray *a) {
    int i2 = a->Int(2);
    HamMove *move = a->Obj<HamMove>(3);
    Symbol s4 = a->ForceSym(4);
    MovePassed(i2, move, s4);
    return 0;
}

void DanceRemixer::PostMoveFinished() {
    int moveIdx = (int)TheTaskMgr.Beat() / 4 + 1;
    UpdateHamDirector();
    MoveDir *moveDir = TheHamDirector->GetMoveDir();
    MoveAsyncDetector *detector = moveDir->GetAsyncDetector();
    for (int i = 0; i < 2; i++) {
        if (ScoredDanceMeasure(i, JumpedMoveIdx(moveIdx - 1) + 1)) {
            detector->DisableAllDetectors();
            break;
        }
    }
    for (int i = 0; i < 2; i++) {
        if (ScoredDanceMeasure(i, moveIdx)) {
            const MoveVariant *mv = TheMoveMgr->mRoutine[i][moveIdx].first;
            if (mv) {
                const char *hamMoveName = mv->GetHamMoveName().Str();
                HamMove *move = moveDir->Find<HamMove>(hamMoveName, false);
                if (move) {
                    detector->EnableDetector(move);
                    moveDir->SetCurrentMove(i, move);
                } else {
                    MILO_NOTIFY(
                        "Ham move %s missing, possibly not loaded yet. From move variant %s",
                        hamMoveName,
                        mv->GetName()
                    );
                }
            }
        }
    }
}

bool DanceRemixer::ScoredDanceMeasure(int player, int measure) const {
    return mUnscoredMeasures[player].find(measure) == mUnscoredMeasures[player].end();
}

void DanceRemixer::UpdateHamDirector() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < TheMoveMgr->mRoutine[i].size(); j++) {
            if (j <= mJumpFromIdx || mJumpToIdx <= j) {
                std::pair<const MoveVariant *, const MoveVariant *> mvs =
                    TheMoveMgr->mRoutine[i][j];
                if (mvs.first) {
                    mVariantsNeededInMemory.insert(mvs.first);
                }
                if (mvs.second && mvs.second != mvs.first) {
                    mVariantsNeededInMemory.insert(mvs.second);
                }
            }
        }
    }
    if (mReloadVariantsNeededInMemory && TheHamDirector->IsMoveMergerFinished()) {
        TheHamDirector->LoadRoutineBuilderData(mVariantsNeededInMemory, true);
        mReloadVariantsNeededInMemory = false;
    }
}

void DanceRemixer::SelectMove(int, int) {}

int DanceRemixer::JumpedMoveIdx(int idx) const { return Round(JumpedBeat(idx * 4)) / 4; }

const MoveParent *DanceRemixer::GetMoveParent(int aPlayer, int aIdx) {
    return TheMoveMgr->mRoutineParents[aPlayer][aIdx];
}

void BuildSetOfPrevAdjacentMoveParents(
    std::set<const MoveParent *> &aFromSet, const std::set<const MoveParent *> &aToSet
) {
    for (std::set<const MoveParent *>::const_iterator it = aToSet.begin();
         it != aToSet.end();
         ++it) {
    }
}

void DanceRemixer::SetUnscoredMeasureRange(
    int player, int first_measure, int last_measure
) {
    for (int i = first_measure; i <= last_measure; i++) {
        mUnscoredMeasures[player].insert(i);
    }
}

void DanceRemixer::ClearUnscoredMeasureRange(
    int player, int first_measure, int last_measure
) {
    for (int i = first_measure; i < last_measure; i++) {
        mUnscoredMeasures[player].erase(i);
    }
}

void DanceRemixer::AddRoutineMove(
    int aPlayer,
    int aMoveIdx,
    const MoveParent *aMove,
    const MoveVariant *aPreferredVariant
) {
    TheMoveMgr->mRoutineParents[aPlayer][aMoveIdx] = aMove;
    TheMoveMgr->mRoutinePreferredVariants[aPlayer][aMoveIdx] = aPreferredVariant;
    TheMoveMgr->FillInRoutineAt(aPlayer, aMoveIdx);
    TheMoveMgr->InsertMoveInSong(
        TheMoveMgr->mRoutine[aPlayer][aMoveIdx].first, aMoveIdx, aPlayer
    );
    MILO_NOTIFY(
        "Jump target to index %d is out of bounds of the song (0 to %d)!",
        mSongMeasures - 1
    );
}
