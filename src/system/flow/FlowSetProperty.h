#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "flow/PropertyEventListener.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "utl/PoolAlloc.h"

class PropertyTask : public Hmx::Object {
public:
    PropertyTask(
        Hmx::Object *,
        DataNode &,
        DataNode &,
        TaskUnits,
        float,
        EaseType,
        float,
        bool,
        Hmx::Object *
    );
    virtual ~PropertyTask();
    OBJ_CLASSNAME(PropertyTask)
    virtual void Poll(float);

    POOL_OVERLOAD(PropertyTask, 0x17)

protected:
    EaseFunc *mEaseFunc;
};

class FlowSetProperty : public FlowNode, public PropertyEventListener {
protected:
    FlowSetProperty(void);
    u32 unk_0x74; // might be fake.
    FlowPtr<Hmx::Object> mTarget;

    DataNode unk_0x98; // "property_path"
    DataNodeObjTrack mValue; // 0xA0
    bool mPersistent; // 0xBC
    int mRate; // 0xC0
    f32 mBlendTime; // 0xC4
    f32 mChangePerUnit; // 0xC8
    ObjOwnerPtr<Task> unk_0xCC;
    int mEase; // 0xE0
    f32 mEasePower; // 0xE4
    u8 unk_0xE8;
    int mStopMode; // 0xEC

    void OnTargetChanged(void);
    void OnAnimEvent(Symbol);
    bool IsBlendable(void);

public:
    virtual ~FlowSetProperty();
    OBJ_CLASSNAME(FlowSetProperty)
    OBJ_SET_TYPE(FlowSetProperty)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual bool Replace(ObjRef *from, Hmx::Object *to);

    virtual bool IsRunning(void);
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual void ChildFinished(FlowNode *);
    virtual void MiloPreRun();
    virtual void UpdateIntensity(void);

    void ReActivate(void);

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(FlowSetProperty)
};
