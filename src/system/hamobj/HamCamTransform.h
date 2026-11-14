#pragma once
#include "flow/Flow.h"
#include "hamobj/HamCamShot.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "world/CameraShot.h"
#include "world/Crowd.h"

class TransformCrowd {
public:
    TransformCrowd(Hmx::Object *owner) : mCrowd(owner), mCrowdRotate(kCrowdRotateNone) {}
    void Save(BinStream &) const;
    void Load(BinStream &);

    /** "The crowd to show for this shot" */
    ObjPtr<WorldCrowd> mCrowd; // 0x0
    /** "How to rotate crowd" */
    CrowdRotate mCrowdRotate; // 0x14
};

class TransformArea {
public:
    TransformArea(Hmx::Object *);
    TransformArea(Hmx::Object *, const TransformArea &);

    void Save(BinStream &) const;
    void Load(BinStreamRev &);

    /** "New origin for area" */
    ObjPtr<RndTransformable> mArea; // 0x0
    /** "Camera shots to be moved to this area" */
    ObjPtrList<HamCamShot> mCamshots; // 0x14
    /** "Anim to be run for these cam shots" */
    ObjPtrList<RndAnimatable> mAnims; // 0x28
    ObjVector<TransformCrowd> mCrowds; // 0x3c
    /** "Flow to execute for the setup of these camshots" */
    ObjPtr<Flow> mFlow; // 0x4c
};

/** "Used to parent camera shots to separate areas.
    This allows camera shots to be shared between venues." */
class HamCamTransform : public RndPollable {
public:
    // Hmx::Object
    virtual ~HamCamTransform();
    OBJ_CLASSNAME(HamCamTransform);
    OBJ_SET_TYPE(HamCamTransform);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x4A)
    NEW_OBJ(HamCamTransform)
    REGISTER_OBJ(HamCamTransform)

    void ClearOldCrowds();

protected:
    HamCamTransform();

    void Setup(bool);

    ObjVector<TransformArea> mAreas; // 0x8
};
