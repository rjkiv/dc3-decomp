#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"

/** "activates children based on comparison cases" */
class FlowSwitch : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowSwitch();
    OBJ_CLASSNAME(FlowSwitch)
    OBJ_SET_TYPE(FlowSwitch)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void ChildFinished(FlowNode *);

    OBJ_MEM_OVERLOAD(0x1C)
    NEW_OBJ(FlowSwitch)

protected:
    FlowSwitch();

    void VerifyTypes();

    /** "The left hand side value of the comparison" */
    DataNode mValue; // 0x5c
    DataNode unk64; // 0x64
    /** "When true, the first case to be valid is the only one activated" */
    bool mFirstValidCaseOnly; // 0x6c
};
