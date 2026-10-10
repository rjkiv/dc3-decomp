#include "hamobj/OriginalChoreoRemixer.h"
#include "OriginalChoreoRemixer.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/DanceRemixer.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/MoveMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"

OriginalChoreoRemixer::OriginalChoreoRemixer() {}

BEGIN_HANDLERS(OriginalChoreoRemixer)
    HANDLE_ACTION(init, Init())
    HANDLE_ACTION(reset, Reset())
    HANDLE(move_passed, OnMovePassed)
    HANDLE_EXPR(desired_difficulty, mDesiredPlayerDifficulty[_msg->Int(2)])
    HANDLE_ACTION(
        set_desired_difficulty,
        mDesiredPlayerDifficulty[_msg->Int(2)] = (Difficulty)_msg->Int(3)
    )
    HANDLE_SUPERCLASS(DanceRemixer)
END_HANDLERS

BEGIN_PROPSYNCS(OriginalChoreoRemixer)
    SYNC_SUPERCLASS(DanceRemixer)
END_PROPSYNCS

BEGIN_SAVES(OriginalChoreoRemixer)
    SAVE_SUPERCLASS(DanceRemixer)
END_SAVES

BEGIN_COPYS(OriginalChoreoRemixer)
    COPY_SUPERCLASS(DanceRemixer)
    CREATE_COPY(OriginalChoreoRemixer)
    BEGIN_COPYING_MEMBERS
        for (int i = 0; i < kNumDifficultiesDC2; i++) {
            COPY_MEMBER(mOriginalMoveVariants[i])
            COPY_MEMBER(mOriginalMoveParents[i])
        }
        for (int i = 0; i < 2; i++) {
            COPY_MEMBER(mDesiredPlayerDifficulty[i])
            COPY_MEMBER(mRoutineDifficulty[i])
        }
        COPY_MEMBER(mIntroMeasures)
        COPY_MEMBER(mFinishingMoveIdx)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(OriginalChoreoRemixer)
    DanceRemixer::Load(bs);
END_LOADS

void OriginalChoreoRemixer::Reset() {
    DanceRemixer::Reset();
    for (int i = 0; i < 2; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        mDesiredPlayerDifficulty[i] = hpd->GetDifficulty();
        for (int j = 0; j < mSongMeasures; j++) {
            SelectMove(i, j);
        }
    }
    UpdateHamDirector();
}

void OriginalChoreoRemixer::PostMoveFinished() {
    static Symbol merge_moves("merge_moves");
    if (TheHamProvider->Property(merge_moves, true)->Int()) {
        int toAdd = JumpedMoveIdxAdd((int)TheTaskMgr.Beat() / 4, 4);
        if (ValidMoveIdx(toAdd)) {
            for (int i = 0; i < 2; i++) {
                SelectMove(i, toAdd);
            }
        }
        DanceRemixer::PostMoveFinished();
    }
}

bool OriginalChoreoRemixer::ScoredDanceMeasure(int x, int y) const {
    int idx = JumpedMoveIdx(y - 1);
    if (idx >= mIntroMeasures && idx <= mFinishingMoveIdx) {
        return DanceRemixer::ScoredDanceMeasure(x, y);
    } else
        return false;
}

void OriginalChoreoRemixer::SelectMove(int player, int measure) {
    if (ValidMoveIdx(measure)) {
        if (JumpedMoveIdx(measure) != measure) {
            MILO_FAIL("selecting move in jump zone. wtf");
        }
        int idx = JumpedMoveIdxAdd(measure, -1);
        int i8;
        if (ValidMoveIdx(idx)) {
            i8 = mRoutineDifficulty[player][idx];
            if (i8 != mDesiredPlayerDifficulty[player]) {
                const MoveParent *mp = GetMoveParent(player, idx);
                if (TheMoveMgr->mWholeMoveGraph.HasVariantPair(
                        mp,
                        GetMoveParentsByDifficulty(
                            mDesiredPlayerDifficulty[player]
                        )[measure]
                    )) {
                    i8 = mDesiredPlayerDifficulty[player];
                }
            }
        } else {
            i8 = mDesiredPlayerDifficulty[player];
        }
        const auto &parents = GetMoveParentsByDifficulty(i8);
        const MoveParent *mp_next = parents[measure];
        const auto &vars = GetMoveVariantsByDifficulty(i8);
        const MoveVariant *mv_next = vars[measure];
        MILO_ASSERT(mp_next, 0xA1);
        AddRoutineMove(player, measure, mp_next, mv_next);
        mRoutineDifficulty[player][measure] = i8;
        DanceRemixer::SelectMove(player, measure);
    }
}

void OriginalChoreoRemixer::Init() {
    if (TheMoveMgr->mWholeMoveGraph.mNodes.size() == 0) {
        TheMoveMgr->InitSong();
        if (TheMoveMgr->mWholeMoveGraph.mNodes.size() == 0) {
            MILO_FAIL("Failed to load move graph for: %s\n", TheGameData->GetSong());
        }
    }
    SaveOriginalMoveParents();
    DanceRemixer::Init(mOriginalMoveParents[0].size());
    for (int i = 0; i < 3; i++) {
        BridgeGapsInMoveParents(i);
    }
    for (int i = 0; i < 2; i++) {
        mRoutineDifficulty[i].clear();
        mRoutineDifficulty[i].resize(mSongMeasures, 999);
    }
}

std::vector<const MoveParent *> &
OriginalChoreoRemixer::GetMoveParentsByDifficulty(int aDiff) {
    MILO_ASSERT(aDiff >= kDifficultyEasy && aDiff < kNumDifficultiesDC2, 0x37);
    return mOriginalMoveParents[aDiff];
}

std::vector<const MoveVariant *> &
OriginalChoreoRemixer::GetMoveVariantsByDifficulty(int aDiff) {
    MILO_ASSERT(aDiff >= kDifficultyEasy && aDiff < kNumDifficultiesDC2, 0x3D);
    return mOriginalMoveVariants[aDiff];
}

void OriginalChoreoRemixer::SaveOriginalMoveParents() {
    mIntroMeasures = mFinishingMoveIdx = -1;
    DataArray *layout = TheMoveMgr->mWholeMoveGraph.mLayoutData;
    Symbol song = TheGameData->GetSong();
    if (!layout) {
        MILO_FAIL("couldn't load layout for: %s\n", song.Str());
    }
    for (int i = 0; i < kNumDifficultiesDC2; i++) {
        Symbol diffSym = DifficultyToSym((Difficulty)i);
        DataArray *a = layout->FindArray(diffSym)->Array(1);
        if (a->Size() == 0) {
            MILO_FAIL(
                "%s's %s layout is not stored in its move graph",
                song.Str(),
                diffSym.Str()
            );
        }
        mOriginalMoveVariants[i].clear();
        mOriginalMoveVariants[i].reserve(a->Size());
        mOriginalMoveParents[i].clear();
        mOriginalMoveParents[i].reserve(a->Size());
        bool b2 = true;
        for (int j = 0; j < a->Size(); j++) {
            Symbol varName = a->Sym(j);
            const MoveVariant *variant =
                TheMoveMgr->mWholeMoveGraph.FindMoveByVariantName(varName);
            const MoveParent *parent = variant ? variant->GetParent() : nullptr;
            MILO_ASSERT((variant==NULL) == (parent==NULL), 0xD4);
            mOriginalMoveVariants[i].push_back(variant);
            mOriginalMoveParents[i].push_back(parent);
            if (variant) {
                if (!variant->IsRest() && b2) {
                    b2 = false;
                    if (mIntroMeasures != -1 && mIntroMeasures != j) {
                        MILO_FAIL(
                            "Different difficulties have different number of intro moves\n"
                        );
                    }
                    mIntroMeasures = j;
                }
                if (variant->IsFinalPose()) {
                    if (mFinishingMoveIdx != -1 && mFinishingMoveIdx != j) {
                        MILO_FAIL(
                            "Different difficulties have final pose at different position\n"
                        );
                    }
                    mFinishingMoveIdx = j;
                    break;
                }
            }
        }
    }
    if (mIntroMeasures == -1 || mFinishingMoveIdx == -1) {
        MILO_FAIL("Remixer could not determine start and end moves in %s", song.Str());
    }
}

void OriginalChoreoRemixer::BridgeGapsInMoveParents(int difficulty) {
    std::vector<std::set<const MoveParent *> > setVec;
    std::vector<const MoveParent *> &moveParentsByDiff =
        GetMoveParentsByDifficulty(difficulty);
    for (int i = 0; i < mSongMeasures; i++) {
        if (!moveParentsByDiff[i]) {
            MILO_ASSERT_FMT(
                i > 0, "MixItMgr: Gap at measure 0 in song %s\n", TheGameData->GetSong()
            );
            int i12 = i + 1;
            for (; i12 < mSongMeasures && !moveParentsByDiff[i12]; i12++) {
            }
            MILO_ASSERT_FMT(
                i12 <= mFinishingMoveIdx,
                "MixItMgr: Gap beyond finishing move in song %s\n",
                TheGameData->GetSong()
            );
            setVec.clear();
            setVec.resize(mSongMeasures);
            setVec[i - 1].insert(moveParentsByDiff[i - 1]);
            setVec[i12].insert(moveParentsByDiff[i12]);
            for (int j = i12 - 1; j >= i; j--) {
                BuildSetOfPrevAdjacentMoveParents(setVec[j], setVec[j + 1]);
            }
            for (int j = i; j < i12; j++) {
                const std::set<const MoveParent *> &curSet = setVec[j];
                const MoveParent *curMoveParentByDiff = moveParentsByDiff[j - 1];
                const MoveParent *found = nullptr;
                FOREACH (it, curSet) {
                    const MoveParent *cur = *it;
                    if (cur->HasPrevAdjacent(curMoveParentByDiff)) {
                        found = cur;
                        break;
                    }
                }
                if (!found) {
                    if (curSet.size() == 0) {
                        if (curMoveParentByDiff->mNext.size() != 0) {
                            found = curMoveParentByDiff->mNext.front();
                        } else {
                            found = curMoveParentByDiff;
                        }
                    } else {
                        found = *curSet.begin();
                    }
                }
                moveParentsByDiff[j] = found;
            }
        }
    }
}
