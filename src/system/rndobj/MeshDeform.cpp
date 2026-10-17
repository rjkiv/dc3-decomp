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
    bs << desc.bone;
    bs << desc.offset << desc.parent;
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
    bs >> desc.bone;
    bs >> desc.offset >> desc.parent;
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
        mVerts.Load(d);
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
        TheDebug << "   " << cur.bone.Ptr() << "\n";
        TheDebug << "   " << cur.offset << "\n";
        TheDebug << "   " << cur.parent << "\n";
    }
    int i = 0;
    for (VertArray::Vert *it = mVerts.Begin(); it < mVerts.End(); it = it->Next(), ++i) {
        TheDebug << "weights" << i << ": ";
        for (int j = 0; j < it->num; j++) {
            TheDebug << "(" << it->weights[j].bone << " " << it->weights[j].GetWeight()
                     << ") ";
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

int RndMeshDeform::VertArray::AppendWeights(
    int num, int *const bones, float *const weights
) {
    MILO_ASSERT(num < VertArray::kMaxWeights, 0x5F);
    int numVerts = NumVerts();
    float weightSum = 0;
    for (int i = 0; i < num; i++) {
        for (int j = i + 1; j < num; j++) {
            if (bones[j] == bones[i]) {
                weights[i] += weights[j];
                num--;
                bones[j] = bones[num];
                weights[j] = weights[num];
                j--;
            }
        }
        if (weights[i] <= 0) {
            MILO_NOTIFY(
                "%s vert %d has negative weight %g on bone, won't export",
                PathName(mParent),
                numVerts,
                weights[i]
            );
            weights[i] = 0;
        }
        weightSum += weights[i];
    }
    if (fabsf(weightSum - 1.0f) > 0.05f) {
        MILO_NOTIFY(
            "%s vert %d weights sum to %g, not close enough to 1, check the skinning",
            PathName(mParent),
            numVerts,
            weightSum
        );
    }
    float div = 1 / weightSum;
    void *mem = MemResizeElem(
        mData,
        mSize,
        End(),
        0,
        num * sizeof(VertArray::Vert::WeightPair) + 1,
        __FILE__,
        0x85,
        "RndMeshDeform"
    );
    Vert *vert = (Vert *)mem;
    vert->num = num;
    for (int i = 0; i < num; i++) {
        vert->weights[i].bone = bones[i];
        vert->weights[i].SetWeight(Clamp(0.0f, 1.0f, weights[i] * div));
    }
    return numVerts;
}
