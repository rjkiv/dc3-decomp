#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Lit.h"
#include "utl/MemMgr.h"

/** "LightAnim objects animate light object properties using keyframe interpolation." */
class RndLightAnim : public RndAnimatable {
public:
    // Hmx::Object
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(LightAnim);
    OBJ_SET_TYPE(LightAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget() { return mLight; }
    virtual void SetKey(float);

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(RndLightAnim)
    REGISTER_OBJ(RndLightAnim)

    Keys<Hmx::Color, Hmx::Color> &ColorKeys() { return mKeysOwner->mColorKeys; }
    RndLightAnim *KeysOwner() const { return mKeysOwner; }
    void SetKeysOwner(RndLightAnim *);

protected:
    RndLightAnim();

    DataNode OnCopyKeys(DataArray *);

    /** "The light whose color we will animate" */
    ObjPtr<RndLight> mLight; // 0x10
    /** "The frames and corresponding color at each frame" */
    Keys<Hmx::Color, Hmx::Color> mColorKeys; // 0x1c
    /** "Owner of the keys, usually myself" */
    ObjOwnerPtr<RndLightAnim> mKeysOwner; // 0x24
};
