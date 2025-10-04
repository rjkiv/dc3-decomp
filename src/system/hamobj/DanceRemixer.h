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
    virtual void Init(int);
    virtual void Reset();
    virtual DataNode OnMovePassed(DataArray *);
    virtual void MovePassed(int, HamMove *, Symbol) {}
    virtual void PostMoveFinished();
    virtual bool ScoredDanceMeasure(int, int) const;

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(DanceRemixer)
    void SetJump(int, int);
    void ClearJump();
    float JumpedBeat(float) const;
    int JumpedMoveIdx(int) const;
    int JumpedMeasureAdd(int, int) const;
    int JumpedMeasureStepsBetween(int, int, int) const;
    void SetUnscoredMeasureRange(int, int, int);
    void ClearUnscoredMeasureRange(int, int, int);
    void SetUnscoredMeasure(int, int);
    void ClearUnscoredMeasure(int, int);
    const MoveVariant *MoveVariantFromHamMove(const HamMove *) const;
    int JumpedMoveIdxAdd(int, int) const;

    bool ValidMoveIdx(int idx) const { return idx >= 0 && idx < mTotalMeasures; }

protected:
    DanceRemixer();

    virtual void UpdateHamDirector();
    virtual void SelectMove(int, int);

    const MoveParent *GetMoveParent(int, int);
    void AddRoutineMove(int, int, const MoveParent *, const MoveVariant *);

    int mTotalMeasures; // 0x2c
    std::set<const MoveVariant *> unk30; // 0x30
    unsigned char unk48; // 0x48
    int mFromMeasure; // 0x4c
    int mToMeasure; // 0x50
    std::map<int, int> unk54; // 0x54
    std::set<int> mUnscoredMeasures[2]; // 0x6c
};

void BuildSetOfPrevAdjacentMoveParents(std::set<const MoveParent *> &, const std::set<const MoveParent *> &);
