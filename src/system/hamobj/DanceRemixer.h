#pragma once
#include "hamobj/HamMove.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include <set>

class DanceRemixer : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~DanceRemixer();
    OBJ_CLASSNAME(DanceRemixer);
    OBJ_SET_TYPE(DanceRemixer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Init(int aNumMeasures);
    virtual void Reset();
    virtual DataNode OnMovePassed(DataArray *msg);
    virtual void MovePassed(int aPlayer, HamMove *aMove, Symbol aRating) {}
    virtual void PostMoveFinished();
    virtual bool ScoredDanceMeasure(int player, int measure) const;

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(DanceRemixer)
    void SetJump(int aFromMeasure, int aToMeasure);
    void ClearJump();
    float JumpedBeat(float b) const;
    int JumpedMoveIdx(int i) const;
    int JumpedMeasureAdd(int measure, int n) const;
    int JumpedMeasureStepsBetween(int, int, int) const;
    void SetUnscoredMeasureRange(int player, int first_measure, int last_measure);
    void ClearUnscoredMeasureRange(int player, int first_measure, int last_measure);
    void SetUnscoredMeasure(int, int);
    void ClearUnscoredMeasure(int, int);
    const MoveVariant *MoveVariantFromHamMove(const HamMove *aHamMove) const;
    int JumpedMoveIdxAdd(int moveIdx, int n) const;

    bool ValidMoveIdx(int idx) const { return idx >= 0 && idx < mSongMeasures; }

protected:
    DanceRemixer();

    virtual void UpdateHamDirector();
    virtual void SelectMove(int aPlayer, int aMoveIdx);

    const MoveParent *GetMoveParent(int aPlayer, int aIdx);
    void AddRoutineMove(
        int aPlayer,
        int aMoveIdx,
        const MoveParent *aMove,
        const MoveVariant *aPreferredVariant
    );

    int mSongMeasures; // 0x2c
    std::set<const MoveVariant *> mVariantsNeededInMemory; // 0x30
    bool mReloadVariantsNeededInMemory; // 0x48
    int mJumpFromIdx; // 0x4c
    int mJumpToIdx; // 0x50
    std::map<int, int> mJumpMap; // 0x54
    std::set<int> mUnscoredMeasures[2]; // 0x6c
};

void BuildSetOfPrevAdjacentMoveParents(
    std::set<const MoveParent *> &, const std::set<const MoveParent *> &
);
