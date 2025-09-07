#pragma once
#include "flow/FlowNode.h"

/** "A value case" */
class FlowValueCase : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowValueCase();
    OBJ_CLASSNAME(FlowValueCase)
    OBJ_SET_TYPE(FlowValueCase)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x1E)
    NEW_OBJ(FlowValueCase)

protected:
    FlowValueCase();

    /** "Key frame value" */
    float mValue; // 0x5c
};
