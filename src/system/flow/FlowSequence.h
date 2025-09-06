#pragma once
#include "flow/FlowNode.h"

/** "A sequence of flow objects" */
class FlowSequence : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowSequence();
    OBJ_CLASSNAME(FlowSequence)
    OBJ_SET_TYPE(FlowSequence)
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
    NEW_OBJ(FlowSequence)

protected:
    FlowSequence();

    int unk5c; // 0x5c
    /** "Loop forever?" */
    bool mLooping; // 0x60
    /** "how many times to reapeat this sequence" */
    int mRepeats; // 0x64
    int unk68; // 0x68
    /** "How should we handle stop requests?" */
    StopMode mStopMode; // 0x6c
    bool unk70;
};
