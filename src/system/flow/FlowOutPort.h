#pragma once
#include "flow/FlowNode.h"
#include "utl/Str.h"

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

protected:
    FlowOutPort();

    String unk5c;
    bool unk64;
    bool unk65;
    bool unk66;
};
