#include "char/CharBonesMeshes.h"
#include "char/CharUtl.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/Str.h"
#include <string.h>

RndTransformable *CharBonesMeshes::sDummyMesh;

CharBonesMeshes::CharBonesMeshes() : mMeshes(this, (EraseMode)0, kObjListOwnerControl) {}

CharBonesMeshes::~CharBonesMeshes() { mMeshes.clear(); }

bool CharBonesMeshes::Replace(ObjRef *from, Hmx::Object *to) {
    ObjPtrVec<RndTransformable>::iterator it = mMeshes.FindRef(from);
    if (it != mMeshes.end()) {
        RndTransformable *trans;
        if (to) {
            trans = dynamic_cast<RndTransformable *>(to);
        } else {
            trans = nullptr;
        }
        mMeshes.Set(it, trans);
        if (!*it) {
            mMeshes.Set(it, sDummyMesh);
        }
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_PROPSYNCS(CharBonesMeshes)
    SYNC_PROP(meshes, mMeshes)
    SYNC_SUPERCLASS(CharBonesObject)
END_PROPSYNCS

void CharBonesMeshes::ReallocateInternal() {
    CharBonesAlloc::ReallocateInternal();
    String str;
    mMeshes.clear();
    mMeshes.reserve(mBones.size());
    for (int i = 0; i < mBones.size(); i++) {
        RndTransformable *trans = CharUtlFindBoneTrans(mBones[i].name.Str(), Dir());
        if (!trans) {
            if (strncmp("bone_facing", mBones[i].name.Str(), 0xB)) {
                str += MakeString("%s, ", mBones[i].name);
            }
            trans = sDummyMesh;
        }
        mMeshes.push_back(trans);
    }
    if (mMeshes.empty()) {
        return;
    } else {
        AcquirePose();
    }
}

void CharBonesMeshes::AcquirePose() {
    ObjPtrVec<RndTransformable>::iterator curMesh = mMeshes.begin();
    Vector3 *vecEnd = PosEnd();
    for (Vector3 *it = GetPos(); it < vecEnd; ++it, ++curMesh) {
        *it = (*curMesh)->LocalXfm().v;
    }
    vecEnd = ScaleEnd();
    for (Vector3 *it = GetScale(); it < vecEnd; ++it, ++curMesh) {
        MakeScale((*curMesh)->LocalXfm().m, *it);
    }
    Hmx::Quat *quatEnd = QuatEnd();
    for (Hmx::Quat *it = GetQuat(); it < quatEnd; ++it, ++curMesh) {
        it->Set((*curMesh)->LocalXfm().m);
    }
    float *rotIt = GetRotX();
    float *rotEnd = GetRotY();
    for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
        *rotIt = GetXAngle((*curMesh)->LocalXfm().m);
    }
    rotEnd = GetRotZ();
    for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
        *rotIt = GetYAngle((*curMesh)->LocalXfm().m);
    }
    rotEnd = GetEnd();
    for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
        *rotIt = GetZAngle((*curMesh)->LocalXfm().m);
    }
}

void CharBonesMeshes::PoseMeshes() {
    ObjPtrVec<RndTransformable>::iterator curMesh = mMeshes.begin();
    Vector3 *vecEnd = PosEnd();
    for (Vector3 *it = GetPos(); it < vecEnd; ++it, ++curMesh) {
        (*curMesh)->SetLocalPos(*it);
    }
    if (mQuatCount < mMeshes.size()) {
        curMesh = mMeshes.begin() + mQuatCount;
        Hmx::Quat *quatEnd = QuatEnd();
        for (Hmx::Quat *it = GetQuat(); it < quatEnd; ++it, ++curMesh) {
            Normalize(*it, *it);
            MakeRotMatrix(*it, (*curMesh)->DirtyLocalXfm().m);
        }
        float *rotIt = GetRotX();
        float *rotEnd = GetRotY();
        for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
            MakeRotMatrixX(*rotIt, (*curMesh)->DirtyLocalXfm().m);
        }
        rotEnd = GetRotZ();
        for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
            MakeRotMatrixY(*rotIt, (*curMesh)->DirtyLocalXfm().m);
        }
        rotEnd = GetEnd();
        for (; rotIt < rotEnd; ++rotIt, ++curMesh) {
            MakeRotMatrixZ(*rotIt, (*curMesh)->DirtyLocalXfm().m);
        }
    }
    if (mScaleCount < mMeshes.size()) {
        curMesh = mMeshes.begin() + mScaleCount;
        Vector3 *vecEnd = ScaleEnd();
        for (Vector3 *it = GetScale(); it < vecEnd; ++it, ++curMesh) {
            Hmx::Matrix3 &mtx = (*curMesh)->DirtyLocalXfm().m;
            Vector3 v;
            MakeScale(mtx, v);
            mtx.x *= it->x / v.x;
            mtx.y *= it->y / v.y;
            mtx.z *= it->z / v.z;
        }
    }
}

void CharBonesMeshes::StuffMeshes(std::list<Hmx::Object *> &oList) {
    for (int i = 0; i < mMeshes.size(); i++) {
        oList.push_back(mMeshes[i]);
    }
}

void CharBonesMeshes::Init() { sDummyMesh = Hmx::Object::New<RndTransformable>(); }

void CharBonesMeshes::Terminate() {}
