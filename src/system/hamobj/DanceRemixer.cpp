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
    return mv ? mv->Name() : "";
}

BEGIN_HANDLERS(DanceRemixer)
    HANDLE_ACTION(reset, Reset())
    HANDLE_ACTION(post_move_finished, PostMoveFinished())
    HANDLE_ACTION(set_jump, SetJump(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(clear_jump, ClearJump())
    HANDLE_EXPR(jump_from_beat, mFromMeasure * 4)
    HANDLE_EXPR(jump_to_beat, mToMeasure * 4)
    HANDLE_EXPR(jump_from_measure, mFromMeasure + 1)
    HANDLE_EXPR(jump_to_measure, mToMeasure + 1)
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
    HANDLE_EXPR(measures_total, mTotalMeasures)
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
        COPY_MEMBER(mTotalMeasures)
        for (int i = 0; i < 2; i++) {
            COPY_MEMBER(mUnscoredMeasures[i])
        }
        COPY_MEMBER(unk30)
        COPY_MEMBER(unk48)
        COPY_MEMBER(mFromMeasure)
        COPY_MEMBER(mToMeasure)
        COPY_MEMBER(unk54)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(DanceRemixer)
    LOAD_SUPERCLASS(Hmx::Object)
END_LOADS

void DanceRemixer::Init(int x) {
    if (TheMoveMgr->MoveParents().size() == 0) {
        MILO_FAIL("Failed to load move graph for: %s\n", TheGameData->GetSong());
    }
    mTotalMeasures = x;
    for (int i = 0; i < 2; i++) {
        TheMoveMgr->mMoveParents[i].resize(mTotalMeasures);
        TheMoveMgr->unk134[i].resize(mTotalMeasures);
        TheMoveMgr->unk150[i].resize(mTotalMeasures);
    }
    ClearJump();
    HandleType(Message("post_init"));
}

void DanceRemixer::Reset() {
    unk30.clear();
    unk48 = false;
    ClearJump();
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < mTotalMeasures; i++) {
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
            const MoveVariant *mv = TheMoveMgr->unk150[i][moveIdx].first;
            if (mv) {
                const char *hamMoveName = mv->HamMoveName().Str();
                HamMove *move = moveDir->Find<HamMove>(hamMoveName, false);
                if (move) {
                    detector->EnableDetector(move);
                    moveDir->SetCurrentMove(i, move);
                } else {
                    MILO_NOTIFY(
                        "Ham move %s missing, possibly not loaded yet. From move variant %s",
                        hamMoveName,
                        mv->Name()
                    );
                }
            }
        }
    }
}

bool DanceRemixer::ScoredDanceMeasure(int x, int y) const {
    return mUnscoredMeasures[x].find(y) == mUnscoredMeasures[x].end();
}

void DanceRemixer::UpdateHamDirector() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < TheMoveMgr->unk150[i].size(); j++) {
            if (j <= mFromMeasure || mToMeasure <= j) {
                std::pair<const MoveVariant *, const MoveVariant *> mvs =
                    TheMoveMgr->unk150[i][j];
                if (mvs.first) {
                    unk30.insert(mvs.first);
                }
                if (mvs.second && mvs.second != mvs.first) {
                    unk30.insert(mvs.second);
                }
            }
        }
    }
    if (unk48 && TheHamDirector->IsMoveMergerFinished()) {
        TheHamDirector->LoadRoutineBuilderData(unk30, true);
        unk48 = false;
    }
}

void DanceRemixer::SelectMove(int, int) {}

int DanceRemixer::JumpedMoveIdx(int idx) const { return Round(JumpedBeat(idx * 4)) / 4; }

const MoveParent *DanceRemixer::GetMoveParent(int x, int y) {
    return TheMoveMgr->CurParents(x)[y];
}

void BuildSetOfPrevAdjacentMoveParents(
    std::set<const MoveParent *> &s1, const std::set<const MoveParent *> &s2
) {
    for (std::set<const MoveParent *>::const_iterator it = s2.begin(); it != s2.end();
         ++it) {
    }
}

void DanceRemixer::SetUnscoredMeasureRange(int x, int y, int z) {
    for (int i = y; i <= z; i++) {
        mUnscoredMeasures[x].insert(i);
    }
}

void DanceRemixer::ClearUnscoredMeasureRange(int x, int y, int z) {
    for (int i = y; i < z; i++) {
        mUnscoredMeasures[x].erase(i);
    }
}

void DanceRemixer::AddRoutineMove(
    int x, int y, const MoveParent *mp, const MoveVariant *mv
) {
    TheMoveMgr->mMoveParents[x][y] = mp;
    TheMoveMgr->unk134[x][y] = mv;
    TheMoveMgr->FillInRoutineAt(x, y);
    TheMoveMgr->InsertMoveInSong(TheMoveMgr->unk150[x][y].first, y, x);
    MILO_NOTIFY(
        "Jump target to index %d is out of bounds of the song (0 to %d)!",
        mTotalMeasures - 1
    );
}
