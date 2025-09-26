#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "rndobj/Trans.h"

/** "Runs children when two trans objects are within a range" */
class FlowDistance : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowDistance();
    OBJ_CLASSNAME(FlowDistance)
    OBJ_SET_TYPE(FlowDistance)
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
    virtual void UpdateIntensity();

    OBJ_MEM_OVERLOAD(0x1D)
    NEW_OBJ(FlowDistance)

protected:
    FlowDistance();

    /** "First object to compare" */
    FlowPtr<RndTransformable> mObj1; // 0x5c
    /** "Second object to compare" */
    FlowPtr<RndTransformable> mObj2; // 0x7c
    /** "Distance for comparison" */
    float mDistance; // 0x9c
    /** "Is the node persistent?" */
    bool mPersistent; // 0xa0
    bool unka1; // 0xa1
    bool unka2; // 0xa2
    /** "Run children when closer than distance value" */
    bool mRunInRange; // 0xa3
    /** "Applies current distance to flow intensity, closer being higher intensity" */
    bool mDriveIntensity; // 0xa4
    float unka8; // 0xa8
};
