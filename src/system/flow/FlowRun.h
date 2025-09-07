#pragma once
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "obj/Dir.h"
#include "utl/Str.h"

/** "Run or stop another Flow" */
class FlowRun : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowRun();
    OBJ_CLASSNAME(FlowRun)
    OBJ_SET_TYPE(FlowRun)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(FlowRun)

    void ResolveTarget();

protected:
    FlowRun();

    void OnTargetDirChange();
    void OnTargetChange();

    /** "Allows you to target flows inside of proxies" */
    FlowPtr<ObjectDir> mTargetDir; // 0x5c
    /** "Flow to start or stop" */
    FlowPtr<Flow> mTarget; // 0x7c
    String mTargetName; // 0x9c
    /** "Stop instead of starting the target flow?" */
    bool mStop; // 0xa4
    /** "If true, we don't track the running state of the target flow" */
    bool mImmediateRelease; // 0xa5
};
