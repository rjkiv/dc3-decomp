#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct HamSupereasyMeasure {
    /** "MoveVariant to use for transition into measure" */
    Symbol first;
    /** "MoveVariant to use for transition out of measure" */
    Symbol second;
    /** "Preferred MoveVariant for this measure" */
    Symbol preferred;
};

/** "Moves for a super easy dance routine" */
class HamSupereasyData : public Hmx::Object {
    friend class SuperEasyRemixer;

public:
    // Hmx::Object
    virtual ~HamSupereasyData();
    OBJ_CLASSNAME(HamSupereasyData);
    OBJ_SET_TYPE(HamSupereasyData);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(HamSupereasyData)

protected:
    HamSupereasyData();

    /** "Routine for Supereasy difficulty." */
    std::vector<HamSupereasyMeasure> mRoutine; // 0x2c
};
