#include "char/CharBonesMeshes.h"
#include "char/CharUtl.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/Str.h"
#include <string.h>

RndTransformable *CharBonesMeshes::sDummyMesh;

CharBonesMeshes::CharBonesMeshes() : mMeshes(this, (EraseMode)0, kObjListOwnerControl) {}

CharBonesMeshes::~CharBonesMeshes() { mMeshes.clear(); }

bool CharBonesMeshes::Replace(ObjRef *ref, Hmx::Object *obj) {
    ObjPtrVec<RndTransformable>::iterator it = mMeshes.FindRef(ref);
    if (it != mMeshes.end()) {
        RndTransformable *trans = dynamic_cast<RndTransformable *>(obj);
        mMeshes.Set(it, trans);
        if (!*it) {
            mMeshes.Set(it, sDummyMesh);
        }
        return true;
    }
    return Hmx::Object::Replace(ref, obj);
}

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
    if (!mMeshes.empty()) {
        AcquirePose();
    }
}

void CharBonesMeshes::AcquirePose() {
    ObjPtrVec<RndTransformable>::iterator curMesh = mMeshes.begin();

    // Copy positions
    char *pos = mStart;
    char *scaleOff = mStart + mOffsets[TYPE_SCALE];
    for (; pos < scaleOff; pos += sizeof(Vector3), ++curMesh) {
        *(Vector3 *)pos = (*curMesh)->LocalXfm().v;
    }

    // Copy scales using MakeScale
    char *quatOff = mStart + mOffsets[TYPE_QUAT];
    for (; pos < quatOff; pos += sizeof(Vector3), ++curMesh) {
        MakeScale((*curMesh)->LocalXfm().m, *(Vector3 *)pos);
    }

    // Copy quaternions using Quat::Set
    char *rotxOff = mStart + mOffsets[TYPE_ROTX];
    for (; pos < rotxOff; pos += sizeof(Hmx::Quat), ++curMesh) {
        ((Hmx::Quat *)pos)->Set((*curMesh)->LocalXfm().m);
    }

    // Copy X rotations
    float *rotIt = (float *)pos;
    float *rotyOff = (float *)(mStart + mOffsets[TYPE_ROTY]);
    for (; rotIt < rotyOff; rotIt++, ++curMesh) {
        *rotIt = GetXAngle((*curMesh)->LocalXfm().m);
    }

    // Copy Y rotations
    float *rotzOff = (float *)(mStart + mOffsets[TYPE_ROTZ]);
    for (; rotIt < rotzOff; rotIt++, ++curMesh) {
        *rotIt = GetYAngle((*curMesh)->LocalXfm().m);
    }

    // Copy Z rotations
    float *endOff = (float *)(mStart + mOffsets[TYPE_END]);
    for (; rotIt < endOff; rotIt++, ++curMesh) {
        *rotIt = GetZAngle((*curMesh)->LocalXfm().m);
    }
}

void CharBonesMeshes::PoseMeshes() {
    ObjPtrVec<RndTransformable>::iterator curMesh = mMeshes.begin();

    // Set positions
    char *pos = mStart;
    char *scaleOff = mStart + mOffsets[TYPE_SCALE];
    for (; pos < scaleOff; pos += sizeof(Vector3), ++curMesh) {
        (*curMesh)->SetLocalPos(*(Vector3 *)pos);
    }

    // Handle quaternions and rotations if we have enough meshes
    if (mCounts[TYPE_QUAT] < mMeshes.size()) {
        curMesh = mMeshes.begin() + mCounts[TYPE_QUAT];

        // Apply quaternion rotations
        char *quatOff = mStart + mOffsets[TYPE_QUAT];
        char *rotxOff = mStart + mOffsets[TYPE_ROTX];
        for (Hmx::Quat *q = (Hmx::Quat *)quatOff; (char *)q < rotxOff;
             q++, ++curMesh) {
            Normalize(*q, *q);
            MakeRotMatrix(*q, (*curMesh)->DirtyLocalXfm().m);
        }

        // Apply X rotations
        float *rotIt = (float *)rotxOff;
        float *rotyOff = (float *)(mStart + mOffsets[TYPE_ROTY]);
        for (; rotIt < rotyOff; rotIt++, ++curMesh) {
            (*curMesh)->DirtyLocalXfm().m.RotateAboutX(*rotIt);
        }

        // Apply Y rotations
        float *rotzOff = (float *)(mStart + mOffsets[TYPE_ROTZ]);
        for (; rotIt < rotzOff; rotIt++, ++curMesh) {
            (*curMesh)->DirtyLocalXfm().m.RotateAboutY(*rotIt);
        }

        // Apply Z rotations
        float *endOff = (float *)(mStart + mOffsets[TYPE_END]);
        for (; rotIt < endOff; rotIt++, ++curMesh) {
            (*curMesh)->DirtyLocalXfm().m.RotateAboutZ(*rotIt);
        }
    }

    // Handle scales if we have enough meshes
    if (mCounts[TYPE_SCALE] < mMeshes.size()) {
        curMesh = mMeshes.begin() + mCounts[TYPE_SCALE];
        char *scaleOff = mStart + mOffsets[TYPE_SCALE];
        char *quatOff = mStart + mOffsets[TYPE_QUAT];
        for (Vector3 *s = (Vector3 *)scaleOff; (char *)s < quatOff;
             s++, ++curMesh) {
            Transform &xfm = (*curMesh)->DirtyLocalXfm();
            Vector3 scale;
            MakeScale(xfm.m, scale);
            xfm.m.x *= s->x / scale.x;
            xfm.m.y *= s->y / scale.y;
            xfm.m.z *= s->z / scale.z;
        }
    }
}

void CharBonesMeshes::StuffMeshes(std::list<Hmx::Object *> &oList) {
    for (int i = 0; i < mMeshes.size(); i++) {
        oList.push_back(mMeshes[i]);
    }
}

BEGIN_PROPSYNCS(CharBonesMeshes)
    SYNC_PROP(meshes, mMeshes)
    SYNC_SUPERCLASS(CharBonesObject)
END_PROPSYNCS

void CharBonesMeshes::Init() { sDummyMesh = Hmx::Object::New<RndTransformable>(); }

void CharBonesMeshes::Terminate() {}
