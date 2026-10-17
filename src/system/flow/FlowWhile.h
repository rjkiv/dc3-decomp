#pragma once
#include "flow/FlowSwitch.h"
#include "flow/PropertyEventListener.h"

/** "A while node; behaves as if constantly evaluting it's property" */
class FlowWhile : public FlowSwitch, public PropertyEventListener {
public:
    // Hmx::Object
    virtual ~FlowWhile();
    OBJ_CLASSNAME(FlowWhile)
    OBJ_SET_TYPE(FlowWhile)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual bool IsRunning();
    virtual void MiloPreRun();

    OBJ_MEM_OVERLOAD(0x1F)
    NEW_OBJ(FlowWhile)

    void UnregisterSelf() { UnregisterEvents(this); }

protected:
    FlowWhile();

    virtual void ChildFinished(FlowNode *);
    // PropertyEventListener
    virtual void GenerateAutoNames(FlowNode *, bool);

    void ReActivate();

    unsigned char mReentranceCount; // 0x88
};
