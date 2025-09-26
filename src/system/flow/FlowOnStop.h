#pragma once
#include "flow/FlowNode.h"

/** "Runs code when a flow is deactivated or requested to stop" */
class FlowOnStop : public FlowNode {
public:
    enum RunMode {
        /** "Only runs when deactivated (hard, forced stop)" */
        kDeactivateOnly = 0,
        /** "Only runs when request stop is called (latent stop)" */
        kRequestStopOnly = 1,
        /** "Fires when deactivated or request stop is called" */
        kAlways = 2
    };
    // Hmx::Object
    virtual ~FlowOnStop();
    OBJ_CLASSNAME(FlowOnStop)
    OBJ_SET_TYPE(FlowOnStop)
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

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(FlowOnStop)

protected:
    FlowOnStop();

    /** "Do we only work on interuption request, or when forced to stop immediately" */
    RunMode mMode; // 0x5c
    bool unk60; // 0x60
};
