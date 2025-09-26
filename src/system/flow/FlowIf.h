#pragma once
#include "flow/FlowNode.h"

class FlowIf : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowIf();
    OBJ_CLASSNAME(FlowIf)
    OBJ_SET_TYPE(FlowIf)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(FlowIf)

protected:
    FlowIf();

    DataNodeObjTrack mValue1; // 0x5c
    DataNodeObjTrack mValue2; // 0x78
    OperatorType mOperator; // 0x94
};
