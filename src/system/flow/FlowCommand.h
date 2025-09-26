#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "obj/Object.h"

/** "Runs a handler exposed by coders on an object" */
class FlowCommand : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowCommand();
    OBJ_CLASSNAME(FlowCommand)
    OBJ_SET_TYPE(FlowCommand)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(FlowCommand)

protected:
    FlowCommand();

    /** "The object which I'm going to trigger a handler on" */
    FlowPtr<Hmx::Object> mObject; // 0x5c
    /** "Handler to fire on the target object" */
    Symbol mHandler; // 0x7c
};
