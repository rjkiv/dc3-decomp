#include "rndobj/MeshDeform.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "math/Rot.h"

#pragma region Hmx::Object

RndMeshDeform::RndMeshDeform()
    : mMesh(this), mBones(this), mVerts(this), mSkipInverse(0), mDeformed(0) {}

RndMeshDeform::~RndMeshDeform() {}

BEGIN_HANDLERS(RndMeshDeform)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMeshDeform)
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP_SET(num_verts, mVerts.NumVerts(), )
    SYNC_PROP_SET(num_bones, (int)mBones.size(), )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void operator<<(BinStream &bs, const RndMeshDeform::BoneDesc &desc) {
    bs << desc.unk0;
    bs << desc.unk14 << desc.unk54;
}

BEGIN_SAVES(RndMeshDeform)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMesh;
    int numBones = mBones.size();
    bs << numBones;
    for (int i = 0; i < numBones; i++) {
        bs << mBones[i];
    }
    mVerts.Save(bs);
    bs << mMeshInverse;
END_SAVES

BEGIN_COPYS(RndMeshDeform)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndMeshDeform)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMesh)
        COPY_MEMBER(mMeshInverse)
        COPY_MEMBER(mBones)
        COPY_MEMBER(mSkipInverse)
        mVerts.Copy(c->mVerts);
    END_COPYING_MEMBERS
END_COPYS

void operator>>(BinStream &bs, RndMeshDeform::BoneDesc &desc) {
    bs >> desc.unk0;
    bs >> desc.unk14 >> desc.unk54;
}

INIT_REVS(1, 0)

BEGIN_LOADS(RndMeshDeform)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mMesh;
    int num = 0;
    if (d.rev < 1) {
        d >> num;
    }
    int numBones;
    d >> numBones;
    if (d.rev < 1) {
        mVerts.Clear();
        int bones[RndMeshDeform::VertArray::kMaxWeights];
        float weights[RndMeshDeform::VertArray::kMaxWeights];
        for (int i = 0; i < num; i++) {
            int weightIdx = 0;
            for (int j = 0; j < numBones; j++) {
                float curWt;
                d >> curWt;
                if (curWt != 0) {
                    bones[weightIdx] = j;
                    weights[weightIdx] = curWt;
                    weightIdx++;
                }
            }
            mVerts.AppendWeights(weightIdx, bones, weights);
        }
    }
    mBones.resize(numBones);
    for (int i = 0; i < numBones; i++) {
        d >> mBones[i];
    }
    if (d.rev > 0) {
        mVerts.Load(d.stream);
    }
    d >> mMeshInverse;
    mSkipInverse = mMeshInverse.v == Vector3(0, 0, 0)
        && mMeshInverse.m.x == Vector3(1, 0, 0) && mMeshInverse.m.y == Vector3(0, 1, 0)
        && mMeshInverse.m.z == Vector3(0, 0, 1);

END_LOADS

void RndMeshDeform::PreSave(BinStream &bs) {
    if (mMesh) {
        mMesh->SetKeepMeshData(true);
    }
}

void RndMeshDeform::Print() {
    TheDebug << "num_verts " << mVerts.NumVerts() << "\n";
    TheDebug << "mesh_inverse " << mMeshInverse << "\n";
    TheDebug << "skip_inverse " << mSkipInverse << "\n";
    TheDebug << "mesh " << mMesh.Ptr() << "\n";
    for (int i = 0; i < mBones.size(); i++) {
        BoneDesc &cur = mBones[i];
        TheDebug << "bone" << i << ":\n";
        TheDebug << "   " << cur.unk0.Ptr() << "\n";
        TheDebug << "   " << cur.unk14 << "\n";
        TheDebug << "   " << cur.unk54 << "\n";
    }
    int i = 0;
    for (auto it = mVerts.begin(); it < mVerts.end(); ++it, ++i) {
        TheDebug << "weights" << i << ": ";

        unsigned char *cData = (unsigned char *)*it;
        for (int j = 0; j < *cData++; j++) {
            unsigned char first = *cData++;
            float second = *cData++ * 0.003921568859368563f;
            TheDebug << "(" << first << " " << second << ") ";
        }
        TheDebug << "\n";
    }
}

#pragma endregion
#pragma region RndMeshDeform

void RndMeshDeform::VertArray::Save(BinStream &bs) {
    bs << mSize;
    bs.Write(mData, mSize);
}

void RndMeshDeform::VertArray::Load(BinStream &bs) {
    int size;
    bs >> size;
    SetSize(size);
    bs.Read(mData, mSize);
}

void RndMeshDeform::VertArray::SetSize(int size) {
    if (mSize != size) {
        mSize = size;
        MemFree(mData);
        mData = MemAlloc(mSize, __FILE__, 0x99, "RndMeshDeform");
    }
}

void RndMeshDeform::VertArray::Copy(const RndMeshDeform::VertArray &a) {
    SetSize(a.mSize);
    memcpy(mData, a.mData, mSize);
}
