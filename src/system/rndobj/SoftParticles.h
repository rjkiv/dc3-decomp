#pragma once
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"

/** "Renders the specified particles with soft edges." */
class RndSoftParticles : public RndDrawable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(SoftParticles);
    OBJ_SET_TYPE(SoftParticles);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);

    OBJ_MEM_OVERLOAD(0x14);
    NEW_OBJ(RndSoftParticles)
    REGISTER_OBJ(RndSoftParticles)

protected:
    RndSoftParticles();

    /** "The list of particle objects to draw" */
    ObjPtrList<RndDrawable> mParticles; // 0x40
    /** "Blending to use for the particle system" */
    BaseMaterial::Blend mBlendMode; // 0x54
};
