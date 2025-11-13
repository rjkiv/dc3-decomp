#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "obj/Task.h"
#include "utl/BinStream.h"

class FlowTimer : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowTimer();
    OBJ_CLASSNAME(FlowTimer)
    OBJ_SET_TYPE(FlowTimer)
    DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(FlowNode::QueueState);
    virtual bool IsRunning();

    void OnKeyframe(FlowNode *);
    void OnTimerEnd();

    int unk5c;
    ObjPtr<Task> unk60;
    int mRate; // 0x74
    float mTotalTime; // 0x78

protected:
    FlowTimer();
};
