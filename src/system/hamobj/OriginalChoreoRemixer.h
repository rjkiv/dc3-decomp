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
    virtual std::vector<const MoveParent *> &GetMoveParentsByDifficulty(int aDiff);
    virtual std::vector<const MoveVariant *> &GetMoveVariantsByDifficulty(int aDiff);

    void SaveOriginalMoveParents();
    void BridgeGapsInMoveParents(int difficulty);

    std::vector<const MoveVariant *> mOriginalMoveVariants[kNumDifficultiesDC2]; // 0x9c
    std::vector<const MoveParent *> mOriginalMoveParents[kNumDifficultiesDC2]; // 0xc0
    // by num players
    Difficulty mDesiredPlayerDifficulty[2]; // 0xe4
    std::vector<int> mRoutineDifficulty[2]; // 0xec
    int mIntroMeasures; // 0x104
    int mFinishingMoveIdx; // 0x108
};
