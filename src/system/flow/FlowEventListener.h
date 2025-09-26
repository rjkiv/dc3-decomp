#pragma once
#include "flow/FlowTrigger.h"

class FlowEventListener : public FlowTrigger {
public:
    // Hmx::Object
    virtual ~FlowEventListener();
    OBJ_CLASSNAME(FlowEventListener)
    OBJ_SET_TYPE(FlowEventListener)
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
    virtual bool IsRunning();
    virtual bool ActivateTrigger();

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(FlowEventListener)

protected:
    FlowEventListener();

    bool unkb4;
    /** "When true, we will start our children when we begin listening for events" */
    bool mStartOnActivate; // 0xb5
    /** "How many events do we process before disabling ourselves? 0 is infinite" */
    int mEventCount; // 0xb8
    int unkbc;
};
