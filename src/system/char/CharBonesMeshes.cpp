#include "char/CharBonesMeshes.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

CharBonesMeshes::CharBonesMeshes() : mMeshes(this, (EraseMode)0, kObjListOwnerControl) {}

CharBonesMeshes::~CharBonesMeshes() { mMeshes.clear(); }

void CharBonesMeshes::ReallocateInternal() {
    CharBonesAlloc::ReallocateInternal();
    mMeshes.clear();
    mMeshes.reserve(mBones.size());
}

BEGIN_PROPSYNCS(CharBonesMeshes)
    SYNC_PROP(meshes, mMeshes)
    SYNC_SUPERCLASS(CharBonesObject)
END_PROPSYNCS

void CharBonesMeshes::Init() { sDummyMesh = Hmx::Object::New<RndTransformable>(); }

void CharBonesMeshes::Terminate() {}

void CharBonesMeshes::StuffMeshes(std::list<Hmx::Object *> &oList) {
    for (int i = 0; i < mMeshes.size(); i++) {
        oList.push_back(mMeshes[i]);
    }
}
