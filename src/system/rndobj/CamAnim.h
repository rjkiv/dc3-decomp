#pragma once
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "obj/Object.h"
#include "math/Key.h"
#include "utl/MemMgr.h"

/**
 * @brief A class for animating Cams.
 * Original _objects description:
 * "A CamAnim object animates Camera properties."
 */
class RndCamAnim : public RndAnimatable {
public:
    // Hmx::Object
    virtual ~RndCamAnim();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CamAnim);
    OBJ_SET_TYPE(CamAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget() { return mCam; }
    virtual void SetKey(float);

    OBJ_MEM_OVERLOAD(0x16);
    NEW_OBJ(RndCamAnim)
    REGISTER_OBJ(RndCamAnim)

    Keys<float, float> &FovKeys() { return mKeysOwner->mFovKeys; }
    RndCamAnim *KeysOwner() const { return mKeysOwner; }

protected:
    RndCamAnim();

    /** The Cam to animate. */
    ObjPtr<RndCam> mCam; // 0x10
    /** The collection of FOV keys. */
    Keys<float, float> mFovKeys; // 0x24
    /** The CamAnim that owns all of these keys. */
    ObjOwnerPtr<RndCamAnim> mKeysOwner; // 0x30
};
