#pragma once
#include "flow/FlowNode.h"

class FlowQueueable : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowQueueable();
    OBJ_CLASSNAME(FlowQueueable)
    OBJ_SET_TYPE(FlowQueueable)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual bool ActivateTrigger() { return FlowNode::Activate(); }
    virtual bool Activate(Hmx::Object *);

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(FlowQueueable)

    void ReleaseListener(Hmx::Object *);

protected:
    FlowQueueable();

    /** "Determines how we handle re-triggering of this label" */
    QueueState mInterrupt; // 0x5c
    std::list<Hmx::Object *> unk60; // 0x60
};
