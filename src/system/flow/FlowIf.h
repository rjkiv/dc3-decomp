#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowSwitchCase.h"

/** "a single case decision node" */
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
    REGISTER_OBJ(FlowIf)

protected:
    FlowIf();

    /** "left hand value in comparison" */
    DataNodeObjTrack mFirstValue; // 0x5c
    /** "right hand value in comparison" */
    DataNodeObjTrack mSecondValue; // 0x78
    /** "equality case to use for comparison" */
    FlowSwitchCase::FlowSwitchCaseComparison mOperator; // 0x94
};
