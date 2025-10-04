#pragma once
#include "hamobj/DanceRemixer.h"
#include "hamobj/Difficulty.h"
#include "hamobj/MoveGraph.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

class OriginalChoreoRemixer : public DanceRemixer {
public:
    // Hmx::Object
    OBJ_CLASSNAME(OriginalChoreoRemixer);
    OBJ_SET_TYPE(OriginalChoreoRemixer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // DanceRemixer
    virtual void Reset();
    virtual void PostMoveFinished();
    virtual bool ScoredDanceMeasure(int, int) const;

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(OriginalChoreoRemixer)

protected:
    OriginalChoreoRemixer();

    virtual void SelectMove(int, int);

public:
    virtual void Init();

protected:
    virtual std::vector<const MoveParent *> &GetMoveParentsByDifficulty(int);
    virtual std::vector<const MoveVariant *> &GetMoveVariantsByDifficulty(int);

    void SaveOriginalMoveParents();
    void BridgeGapsInMoveParents(int);

    std::vector<const MoveVariant *> mMoveVariantsByDiff[kNumDifficultiesDC2]; // 0x9c
    std::vector<const MoveParent *> mMoveParentsByDiff[kNumDifficultiesDC2]; // 0xc0
    // by num players
    Difficulty mDesiredDiffs[2]; // 0xe4
    std::vector<int> unkec[2]; // 0xec

    int unk104; // 0x104 - start move idx?
    int unk108; // 0x108 - end move idx?
};
