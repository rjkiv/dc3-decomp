#pragma once
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "flow/PropertyEventListener.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "utl/PoolAlloc.h"

class PropertyTask : public Task {
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
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(PropertyTask)
    virtual void Poll(float);

    POOL_OVERLOAD(PropertyTask, 0x17)

protected:
    void SetProperty(DataNode &);

    ObjOwnerPtr<Hmx::Object> mTarget; // 0x2c
    DataNode mProp; // 0x40
    DataNode mTargetValue; // 0x48
    DataNode mStartValue; // 0x50
    float mTime; // 0x58
    float mEasePower; // 0x5c
    bool mUseColorInterp; // 0x60
    ObjPtr<Hmx::Object> mEventReciever; // 0x64
    DataType mTargetType; // 0x78
    EaseFunc *pt2EaseFuncion; // 0x7c
};

class FlowSetProperty : public FlowNode, public PropertyEventListener {
public:
    virtual ~FlowSetProperty();
    virtual bool Replace(ObjRef *from, Hmx::Object *to);
    OBJ_CLASSNAME(FlowSetProperty)
    OBJ_SET_TYPE(FlowSetProperty)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual bool IsRunning(void);
    virtual void MiloPreRun();
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual void UpdateIntensity(void);

    void ReActivate(void);

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(FlowSetProperty)

protected:
    FlowSetProperty();

    u32 unk_0x74; // might be fake.
    FlowPtr<Hmx::Object> mTargetObject; // 0x78
    DataNode mProperty; // 0x98
    DataNodeObjTrack mValue; // 0xA0
    bool mRegisterPEPs; // 0xBC
    RndAnimatable::Rate mRate; // 0xC0
    float mBlendTime; // 0xC4
    float mChangePerUnit; // 0xC8
    ObjOwnerPtr<Task> mSpawnedTask; // 0xcc
    EaseType mEase; // 0xE0
    float mEasePower; // 0xE4
    bool mUseColorInterp; // 0xe8
    StopMode mStopMode; // 0xEC

    void OnTargetChanged(void);
    void OnAnimEvent(Symbol);
    bool IsBlendable(void);
};
