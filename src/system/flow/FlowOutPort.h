#pragma once
#include "flow/FlowNode.h"
#include "utl/Str.h"

/** "An output port for flow encapsulation" */
class FlowOutPort : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowOutPort();
    OBJ_CLASSNAME(FlowOutPort)
    OBJ_SET_TYPE(FlowOutPort)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowQueueable
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();

    OBJ_MEM_OVERLOAD(0x2D)
    NEW_OBJ(FlowOutPort)

protected:
    FlowOutPort();

    void UpdatePortMapping();

    /** "Name to show when encapsulated" */
    String mLabel; // 0x5c
    /** "do not wait around for the label to finish?" */
    bool mImmediateRelease; // 0x64
    /** "When true, we request the label to stop instead of running it" */
    bool mStop; // 0x65
    /** "Is this port exposed for possible linking?" */
    bool mExposed; // 0x66
};
