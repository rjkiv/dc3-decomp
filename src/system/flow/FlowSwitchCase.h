#pragma once
#include "flow/FlowNode.h"

/** "a case for a flow switch" */
class FlowSwitchCase : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowSwitchCase();
    OBJ_CLASSNAME(FlowSwitchCase)
    OBJ_SET_TYPE(FlowSwitchCase)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual bool IsRunning();

    OBJ_MEM_OVERLOAD(0x24)
    NEW_OBJ(FlowSwitchCase)

protected:
    FlowSwitchCase();

    void UseLastValueChanged();

    /** "the value we're transitioning to" */
    DataNodeObjTrack mToValue; // 0x5c
    /** "the value we're transitioning from" */
    DataNodeObjTrack mFromValue; // 0x78
    /** "equality case to use for comparison" */
    OperatorType mOperator; // 0x94
    /** "Use the last value to compare against" */
    bool mUseLastValue; // 0x98
    bool mUnregisterParent; // 0x99
    bool unk9a; // 0x9a
};
