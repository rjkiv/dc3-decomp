#pragma once

#include "obj/Object.h"
#include "obj/Object.h"

class FlowNode : public virtual Hmx::Object {
private:
    bool mDebugOutput; // 0x8
    String mDebugComment; // 0xc
    ObjPtrVec<FlowNode> mVec1; // 0x14
    ObjPtrList<FlowNode> mChildren; // 0x30
    FlowNode *mOwner; // 0x44; ?

protected:
    FlowNode();
    static bool sPushDrivenProperties;
    static float sIntensity;
    void ActivateChild(FlowNode *);
    void PushDrivenProperties(void);

public:
    virtual ~FlowNode();
    OBJ_CLASSNAME(FlowNode)
    OBJ_SET_TYPE(FlowNode)
    virtual void MiloPreRun(void);
    virtual void ChildFinished(FlowNode *);
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual bool IsRunning();
    virtual void SetParent(FlowNode *, bool);

    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);

    static FlowNode *DuplicateChild(FlowNode *);
};
