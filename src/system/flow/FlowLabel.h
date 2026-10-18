#pragma once
#include "flow/FlowQueueable.h"
#include "os/Debug.h"

/** "A label for flow encapsulation" */
class FlowLabel : public FlowQueueable {
public:
    // Hmx::Object
    virtual ~FlowLabel();
    OBJ_CLASSNAME(FlowLabel)
    OBJ_SET_TYPE(FlowLabel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate() {
        MILO_ASSERT(false, 0x25);
        return false;
    }

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(FlowLabel)
    REGISTER_OBJ(FlowLabel)

    bool Activate(FlowNode *);
    Symbol GetLabel() { return mLabel; }

protected:
    FlowLabel();

    Symbol mLabel; // 0x68
};
