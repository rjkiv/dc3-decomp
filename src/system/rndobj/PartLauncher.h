#pragma once
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "rndobj/Part.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"

class RndPartLauncher : public RndPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(PartLauncher);
    OBJ_SET_TYPE(PartLauncher);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x13)
    NEW_OBJ(RndPartLauncher)
    REGISTER_OBJ(RndPartLauncher)
    void LaunchParticles();

protected:
    RndPartLauncher();

    void SetBit(unsigned int bitmask, bool high) {
        if (high)
            mPartOverride.mask |= bitmask;
        else
            mPartOverride.mask &= ~bitmask;
    }

    void CopyPropsFromPart();

    ObjPtr<RndParticleSys> mPart; // 0x8
    ObjPtr<RndTransformable> mTrans; // 0x1c
    ObjPtr<RndMesh> mMeshEmitter; // 0x30
    int mNumParts; // 0x44
    Vector2 mEmitRate; // 0x48
    float mEmitCount; // 0x50
    PartOverride mPartOverride; // 0x54
};
