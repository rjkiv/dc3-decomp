#pragma once
#include "obj/Object.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

class RndWind : public RndHighlightable {
public:
    virtual ~RndWind();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Wind);
    OBJ_SET_TYPE(Wind);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Highlight() {}

    OBJ_MEM_OVERLOAD(0x1A);

    void SetWindOwner(RndWind *wind) { mWindOwner = wind ? wind : this; }
    // void Zero();
    // void SetDefaults();
    // float GetWind(float);
    // void GetWind(const Vector3 &v, float f, Vector3 &v2) {
    //     return mWindOwner->SelfGetWind(v, f, v2);
    // }
    // void SelfGetWind(const Vector3 &, float, Vector3 &);
protected:
    RndWind();
    void SyncLoops();

    Vector3 mPrevailing; // 0x8
    Vector3 mRandom; // 0x18
    float mTimeLoop; // 0x28
    float mSpaceLoop; // 0x2c
    ObjPtr<RndTransformable> mTrans; // 0x30
    bool mAboutZ; // 0x44
    float mMaxSpeed; // 0x48
    float mMinSpeed; // 0x4c
    Vector3 mTimeRate; // 0x50
    Vector3 mSpaceRate; // 0x60
    ObjOwnerPtr<RndWind> mWindOwner; // 0x70
};
