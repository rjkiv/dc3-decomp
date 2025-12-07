#pragma once
#include "char/CharBones.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

/** "Holds state for a set of bones, allocates own space, and sets meshes accordingly" */
class CharBonesMeshes : public CharBonesAlloc {
public:
    CharBonesMeshes();
    virtual ~CharBonesMeshes();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void AcquirePose();
    void PoseMeshes();
    void StuffMeshes(std::list<Hmx::Object *> &);

    static void Init();
    static void Terminate();

protected:
    virtual void ReallocateInternal();

    static RndTransformable *sDummyMesh;

    /** "Transes we will change" */
    ObjPtrVec<RndTransformable> mMeshes; // 0x58
};
