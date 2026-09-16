#include "rndobj/AmbientOcclusion.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.inl"
#include "math/kdTree.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
#include "rndobj/Utl.h"
#include "world/Instance.h"
#include <float.h>

bool IsValidObject(Hmx::Object *obj) {
    RndMesh *mesh = dynamic_cast<RndMesh *>(obj);
    RndGroup *group = dynamic_cast<RndGroup *>(obj);
    return mesh || group || dynamic_cast<WorldInstance *>(obj);
}

template <class T>
unsigned int GatherObjectsFromDir(ObjectDir *dir, std::vector<T *> &objects) {
    RndDir *rDir = dynamic_cast<RndDir *>(dir);
    bool showing = rDir ? rDir->Showing() : true;
    if (showing) {
        for (ObjDirItr<Hmx::Object> it(dir, true); it != NULL; ++it) {
            ObjectDir *curDir = dynamic_cast<ObjectDir *>(&*it);
            if (curDir && curDir != dir
                && dynamic_cast<WorldInstance *>((Hmx::Object *)curDir)) {
                GatherObjectsFromDir(curDir, objects);
            }
            T *curObj = dynamic_cast<T *>(&*it);
            if (curObj) {
                objects.push_back(curObj);
            }
        }
    }
    return objects.size();
}

template <class T>
unsigned int GatherObjectsFromGroup(RndGroup *group, std::vector<T *> &objects) {
    if (group->Showing()) {
        std::list<RndDrawable *> draws;
        group->ListDrawChildren(draws);
        FOREACH (it, draws) {
            RndGroup *curGroup = dynamic_cast<RndGroup *>(*it);
            if (curGroup && curGroup != group) {
                GatherObjectsFromGroup(curGroup, objects);
            }
            ObjectDir *curDir = dynamic_cast<ObjectDir *>(*it);
            if (curDir && dynamic_cast<WorldInstance *>((Hmx::Object *)curDir)) {
                GatherObjectsFromDir(curDir, objects);
            }
            RndMesh *curMesh = dynamic_cast<RndMesh *>(*it);
            if (curMesh) {
                objects.push_back(curMesh);
            }
        }
    }
    return objects.size();
}

template <class T>
unsigned int GatherObject(Hmx::Object *object, std::vector<T *> &objects) {
    MILO_ASSERT(IsValidObject(object), 0xD1);
    T *templateObj = dynamic_cast<T *>(object);
    if (templateObj) {
        objects.push_back(templateObj);
    } else {
        RndGroup *group = dynamic_cast<RndGroup *>(object);
        if (group) {
            GatherObjectsFromGroup(group, objects);
        } else {
            ObjectDir *dir = dynamic_cast<ObjectDir *>(object);
            if (dir) {
                GatherObjectsFromDir(dir, objects);
            }
        }
    }
    return objects.size();
}

RndAmbientOcclusion::RndAmbientOcclusion()
    : mDontCastAO(this), mDontReceiveAO(this), mTessellate(this),
      mIgnoreTransparent(true), mIgnorePrelit(true), mIgnoreHidden(true),
      mUseMeshNormals(true), mIntersectBackFaces(false), mTessellateTriLimit(8),
      mTessellateTriError(0.67625f), mTessellateTriLarge(gUnitsPerMeter * 2.0f),
      mTessellateTriSmall(gUnitsPerMeter * 0.5f), mTree(0), mQuality((Quality)1) {}

RndAmbientOcclusion::~RndAmbientOcclusion() { Clean(); }

BEGIN_HANDLERS(RndAmbientOcclusion)
    HANDLE(get_valid_objects, OnGetValidObjects)
    HANDLE(get_recv_meshes, OnGetRecvMeshes)
    HANDLE_ACTION(
        calculate, _msg->Size() > 2 ? OnCalculate(_msg->Int(2)) : OnCalculate(true)
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndAmbientOcclusion)
    SYNC_PROP(dont_cast_ao, mDontCastAO)
    SYNC_PROP(dont_receive_ao, mDontReceiveAO)
    SYNC_PROP(tessellate, mTessellate)
    SYNC_PROP(ignore_transparent, mIgnoreTransparent)
    SYNC_PROP(ignore_prelit, mIgnorePrelit)
    SYNC_PROP(ignore_hidden, mIgnoreHidden)
    SYNC_PROP(use_mesh_normals, mUseMeshNormals)
    SYNC_PROP(intersect_back_faces, mIntersectBackFaces)
    SYNC_PROP(tessellate_tri_limit, mTessellateTriLimit)
    SYNC_PROP(tessellate_tri_error, mTessellateTriError)
    SYNC_PROP(tessellate_tri_large, mTessellateTriLarge)
    SYNC_PROP(tessellate_tri_small, mTessellateTriSmall)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndAmbientOcclusion)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mDontReceiveAO;
    bs << mDontCastAO;
    bs << mTessellate;
    bs << mIgnoreTransparent;
    bs << mIgnorePrelit;
    bs << mIgnoreHidden;
    bs << mUseMeshNormals;
    bs << mIntersectBackFaces;
    bs << mTessellateTriLimit;
    bs << mTessellateTriError;
    bs << mTessellateTriLarge;
    bs << mTessellateTriSmall;
    bs << mQuality;
END_SAVES

BEGIN_COPYS(RndAmbientOcclusion)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndAmbientOcclusion)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDontReceiveAO)
        COPY_MEMBER(mDontCastAO)
        COPY_MEMBER(mTessellate)
        COPY_MEMBER(mIgnoreTransparent)
        COPY_MEMBER(mIgnorePrelit)
        COPY_MEMBER(mIgnoreHidden)
        COPY_MEMBER(mUseMeshNormals)
        COPY_MEMBER(mIntersectBackFaces)
        COPY_MEMBER(mTessellateTriLimit)
        COPY_MEMBER(mTessellateTriError)
        COPY_MEMBER(mTessellateTriLarge)
        COPY_MEMBER(mTessellateTriSmall)
        COPY_MEMBER(mQuality)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BEGIN_LOADS(RndAmbientOcclusion)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mDontReceiveAO;
    d >> mDontCastAO;
    d >> mTessellate;
    d >> mIgnoreTransparent;
    d >> mIgnorePrelit;
    d >> mIgnoreHidden;
    d >> mUseMeshNormals;
    if (d.rev > 3) {
        d >> mIntersectBackFaces;
    }
    if (d.rev > 1) {
        d >> mTessellateTriLimit;
        d >> mTessellateTriError;
        d >> mTessellateTriLarge;
        d >> mTessellateTriSmall;
    }
    if (d.rev > 2) {
        d >> (int &)mQuality;
    }
END_LOADS

static const unsigned int sNumSamples[] = { 0x12C, 0x96 };
static const kdTree<Triangle>::SplitPlaneType sSplitTypes[] = {
    kdTree<Triangle>::kSplitPlane_SAH, kdTree<Triangle>::kSplitPlane_Mean
};

void RndAmbientOcclusion::BuildTrees(Quality quality) {
    MILO_ASSERT(quality < kQuality_Max, 0x1E3);
    mQuality = quality;
    if (!mObjectsCast.empty() && !mObjectsReceive.empty()) {
        MILO_ASSERT(mTriList.empty(), 0x1E9);
        Timer timer;
        timer.Restart();
        MILO_LOG("RndAmbientOcclusion: Building kd-Tree...\n");
        kdTree<Triangle>::SplitPlaneType whichType = sSplitTypes[quality];
        BuildSphereStratified(sNumSamples[quality], unkb8);
        Vector3 maxNeg(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        Vector3 maxPos(FLT_MAX, FLT_MAX, FLT_MAX);
        Box box(maxPos, maxNeg);
        FOREACH (it, mObjectsCast) {
            RndMesh *cur = *it;
            const Transform &curWorldXfm = cur->WorldXfm();
            for (int i = 0; i < cur->Faces().size(); i++) {
                RndMesh::Face &face = cur->Faces(i);
                Vector3 v11a0;
                Multiply(cur->Verts(face.v1).pos, curWorldXfm, v11a0);
                Vector3 v1190;
                Multiply(cur->Verts(face.v2).pos, curWorldXfm, v1190);
                Vector3 v1180;
                Multiply(cur->Verts(face.v3).pos, curWorldXfm, v1180);

                Vector3 diff23;
                Subtract(v1190, v1180, diff23);
                float len23 = Length(diff23);
                Vector3 diff13;
                Subtract(v11a0, v1180, diff13);
                float len13 = Length(diff13);
                Vector3 diff12;
                Subtract(v11a0, v1190, diff12);
                float len12 = Length(diff12);

                if (0.000099999997f < len23 + len13 + len12
                    && 1.1920929E-7f < len23 * len13 * len12) {
                    box.GrowToContain(v11a0, false);
                    box.GrowToContain(v1190, false);
                    box.GrowToContain(v1180, false);
                    Triangle tri;
                    tri.Set(v11a0, v1190, v1180);
                    mTriList.push_back(tri);
                    if (mIntersectBackFaces) {
                        tri.Set(v11a0, v1180, v1190);
                        mTriList.push_back(tri);
                    }
                }
            }
        }
        MILO_ASSERT(mTree == NULL, 0x234);
        box.Extend(0.001f);
        mTree = new kdTree<Triangle>(box);
        FOREACH (it, mTriList) {
            mTree->Insert(&*it);
        }
        mTree->PackNodes(whichType, 0);
        MILO_LOG(
            "RndAmbientOcclusion: Built kd-Tree in %0.2f seconds\n",
            timer.SplitMs() * 0.001f
        );
        timer.Restart();
    }
    DumpObjList(" RndAmbientOcclusion: Cast List:\n", mObjectsCast);
    DumpObjList(" RndAmbientOcclusion: Recv List:\n", mObjectsReceive);
    DumpObjList(" RndAmbientOcclusion: Tess List:\n", mObjectsTessellate);
    mObjectsCast.clear();
}

void RndAmbientOcclusion::Clean() {
    RELEASE(mTree);
    mObjectsCast.clear();
    mObjectsReceive.clear();
    mObjectsTessellate.clear();
    mTriList.clear();
    unkb8.clear();
}

bool RndAmbientOcclusion::IsValid_AOCast(const RndMesh *mesh) const {
    bool b2 = false;
    if (!IsValid_Mesh(mesh)) {
        return false;
    }
    RndMat *mat = mesh->Mat();
    if (mat) {
        ZMode zMode = mat->GetZMode();
        b2 = zMode == kZModeDisable || zMode == kZModeTransparent;
    }
    if ((!mIgnoreHidden || mesh->Showing()) && (!mIgnoreTransparent || !b2)) {
        return true;
    }
    return false;
}

bool RndAmbientOcclusion::IsValid_AOReceive(const RndMesh *mesh) const {
    bool b2 = false;
    bool c4 = false;
    if (!IsSerializable(mesh)) {
        return false;
    }
    if (!IsValid_Mesh(mesh)) {
        return false;
    }

    RndMat *mat = mesh->Mat();
    if (mat) {
        ZMode zMode = mat->GetZMode();
        b2 = zMode == kZModeDisable || zMode == kZModeTransparent || mat->Alpha() == 0;
        c4 = mat->PreLit();
    }
    if ((!mIgnoreHidden || mesh->Showing()) && (!mIgnoreTransparent || !b2)
        && (!mIgnorePrelit || !c4)) {
        return true;
    }
    return false;
}

bool RndAmbientOcclusion::IsValid_Tessellate(
    const RndMesh *mesh, const ObjectDir *dir
) const {
    return IsValid_AOCast(mesh) && IsValid_AOReceive(mesh) && !mesh->IsSkinned()
        && mesh->GetGeomOwner() == mesh && mesh->Dir() != dir;
}

void RndAmbientOcclusion::BuildSHCoeff(const Vector3 &inVector, float *fArr) const {
    MILO_ASSERT(Abs(1.0f - Length(inVector)) <= kSmallFloat, 0x298);
    fArr[0] = 0.2820948f;
    fArr[1] = inVector.y * 0.48860252f;
    fArr[2] = inVector.z * 0.48860252f;
    fArr[3] = inVector.x * 0.48860252f;
}

template <class T>
struct VectorSort {
    VectorSort(const std::vector<T> &v) : mRefList(v) {}

    bool operator()(T item1, T item2) {
        int diff1 = std::find(mRefList.begin(), mRefList.end(), item1) - mRefList.begin();
        int diff2 = std::find(mRefList.begin(), mRefList.end(), item2) - mRefList.begin();
        return diff1 < diff2;
    }

    const std::vector<T> &mRefList; // 0x0
};

void RndAmbientOcclusion::BuildObjectLists() {
    ObjectDir *myDir = Dir();
    Clean();
    MILO_ASSERT(mObjectsCast.empty(), 0x199);
    MILO_ASSERT(mObjectsReceive.empty(), 0x19A);
    MILO_ASSERT(mObjectsTessellate.empty(), 0x19B);
    std::vector<RndMesh *> meshes;
    GatherObjectsFromDir(myDir, meshes);
    std::unique(meshes.begin(), meshes.end());
    std::vector<RndMesh *> dontCastMeshes;
    std::vector<RndMesh *> dontReceiveMeshes;
    std::vector<RndMesh *> tessellateMeshes;
    FOREACH (it, mDontCastAO) {
        GatherObject(*it, dontCastMeshes);
    }
    FOREACH (it, mDontReceiveAO) {
        GatherObject(*it, dontReceiveMeshes);
    }
    FOREACH (it, mTessellate) {
        GatherObject(*it, tessellateMeshes);
    }
    std::unique(dontCastMeshes.begin(), dontCastMeshes.end());
    std::unique(dontReceiveMeshes.begin(), dontReceiveMeshes.end());
    std::unique(tessellateMeshes.begin(), tessellateMeshes.end());
    FOREACH (it, meshes) {
        RndMesh *cur = *it;
        if (IsValid_AOCast(cur)
            && std::find(dontCastMeshes.begin(), dontCastMeshes.end(), cur)
                == dontCastMeshes.end()) {
            mObjectsCast.push_back(cur);
        }
        if (IsValid_AOReceive(cur)
            && std::find(dontReceiveMeshes.begin(), dontReceiveMeshes.end(), cur)
                == dontReceiveMeshes.end()) {
            mObjectsReceive.push_back(cur);
        }
        if (IsValid_Tessellate(cur, myDir)
            && std::find(tessellateMeshes.begin(), tessellateMeshes.end(), cur)
                != tessellateMeshes.end()) {
            mObjectsTessellate.push_back(cur);
        }
    }
    std::sort(
        mObjectsTessellate.begin(),
        mObjectsTessellate.end(),
        VectorSort<RndMesh *>(tessellateMeshes)
    );
}

void RndAmbientOcclusion::TransformNormal(
    const Vector3 &vin, const Hmx::Matrix3 &min, Vector3 &vout
) const {
    Vector3 vtmp;
    Normalize(vin, vtmp);
    Hmx::Matrix3 mtmp;
    Invert(min, mtmp);
    Transpose(mtmp, mtmp);
    Multiply(vtmp, mtmp, vout);
    Normalize(vout, vout);
}

void RndAmbientOcclusion::DumpObjList(
    const char *msg, const std::vector<RndMesh *> &meshes
) const {
    if (!meshes.empty()) {
        MILO_LOG(msg);
        FOREACH (it, meshes) {
            RndMesh *cur = *it;
            const char *statsMsg = MakeString(
                "   %s - %d verts, %d polys\n",
                cur->Name(),
                cur->Verts().size(),
                cur->Faces().size()
            );
            MILO_LOG(statsMsg);
        }
    }
}

bool RndAmbientOcclusion::IsSerializable(const RndMesh *mesh) const {
    if (mesh->GetGeomOwner() != mesh) {
        return false;
    }
    if (mesh->Dir() == Dir()) {
        return true;
    } else {
        ObjectDir *meshDir = mesh->Dir();
        return (meshDir->IsSubDir() && meshDir->InlineSubDirType() == kInlineAlways);
    }
}

bool RndAmbientOcclusion::IsValid_Mesh(const RndMesh *mesh) const {
    RndMesh *nonConstMesh = (RndMesh *)mesh; // lmao
    if (nonConstMesh->Verts().size() && nonConstMesh->Faces().size()) {
        static Symbol classNames[] = { "Spotlight", "WorldCrowd" };
        FOREACH_OBJREF (it, mesh) {
            Hmx::Object *owner = it->RefOwner();
            if (owner) {
                for (int i = 0; i < DIM(classNames); i++) {
                    if (owner->ClassName() == classNames[i]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

bool RndAmbientOcclusion::IsMeshAnimated(const RndMesh *mesh) const {
    static Symbol sRndTransAnim = RndTransAnim::StaticClassName();
    static Symbol sRndPropAnim = RndPropAnim::StaticClassName();
    static DataArrayPtr sPropPathScale(Symbol("scale"));
    static DataArrayPtr sPropPathRotation(Symbol("rotation"));
    FOREACH_OBJREF (it, mesh) {
        Hmx::Object *owner = it->RefOwner();
        if (owner) {
            if (owner->ClassName() == sRndTransAnim) {
                return true;
            }
            if (owner->ClassName() == sRndPropAnim) {
                RndPropAnim *propAnim = dynamic_cast<RndPropAnim *>(owner);
                MILO_ASSERT(propAnim != NULL, 0x7C4);
                if (propAnim->GetKeys(mesh, sPropPathScale)) {
                    return true;
                }
                if (propAnim->GetKeys(mesh, sPropPathRotation)) {
                    return true;
                }
            }
        }
    }
    return false;
}

float RndAmbientOcclusion::DistanceSH(
    const Vector4 &v1, const Vector3 &v2, const Vector4 &v3, const Vector3 &v4
) const {
    float x = v1.x - v3.x;
    float y = (v1.y * 2 - 1) - (v3.y * 2 - 1);
    float z = (v1.z * 2 - 1) - (v3.z * 2 - 1);
    float w = (v1.w * 2 - 1) - (v3.w * 2 - 1);
    float dot = Dot(v2, v4);
    float len = sqrtf(x * x + y * y + z * z + w * w);
    if (dot <= 0) {
        dot = -dot;
    }
    return len / (dot + 1);
}

bool RndAmbientOcclusion::CanBurnXfm(const RndMesh *mesh) const {
    if (IsMeshAnimated(mesh))
        return false;
    else {
        FOREACH (it, mesh->Children()) {
            RndMesh *cur = dynamic_cast<RndMesh *>(*it);
            if (cur) {
                if (!CanBurnXfm(cur)) {
                    return false;
                }
            }
        }
        return true;
    }
}

void RndAmbientOcclusion::PreprocessMesh() {
    std::list<RndMesh *> meshes;
    FOREACH (it, mObjectsReceive) {
        RndMesh *cur = *it;
        cur->UpdateSphere();
        meshes.push_back(cur);
    }
    FOREACH (it, mObjectsReceive) {
        BurnTransform(*it, meshes);
    }
}

void RndAmbientOcclusion::OnCalculate(bool b1) {
    float f1 = 0;
    float f2 = 0;
    float f3 = 0;
    BuildObjectLists();
    BuildTrees((Quality)0);
    CalculateAO(&f1);
    Tessellate(&f2, &f3);
    Clean();
}

DataNode RndAmbientOcclusion::OnGetRecvMeshes(DataArray *) {
    BuildObjectLists();
    unsigned int numReceives = mObjectsReceive.size();
    DataArrayPtr ptr(new DataArray(numReceives));
    for (int i = 0; i < numReceives; i++) {
        ptr->Node(i) = mObjectsReceive[i];
    }
    Clean();
    return ptr;
}

DataNode RndAmbientOcclusion::OnGetValidObjects(DataArray *) const {
    int numObjects = 0;
    for (ObjDirItr<Hmx::Object> it(Dir(), true); it != NULL; ++it) {
        if (IsValidObject(it) && it != Dir()) {
            numObjects++;
        }
    }
    DataArrayPtr ptr(new DataArray(numObjects));
    int idx = 0;
    for (ObjDirItr<Hmx::Object> it(Dir(), true); it != NULL; ++it) {
        if (IsValidObject(it) && it != Dir()) {
            ptr->Node(idx++) = &*it;
        }
    }
    return ptr;
}

void RndAmbientOcclusion::BlendVert(
    const RndMesh::Vert &v1, const RndMesh::Vert &v2, RndMesh::Vert &v3
) {
    v3 = v1;
    Add(v3.pos, v2.pos, v3.pos);
    v3.tex += v2.tex;
    Add(v3.color, v2.color, v3.color);
    Add(v3.norm, v2.norm, v3.norm);
    Vector3 tangent = reinterpret_cast<Vector3 &>(v3.tangent);
    tangent.x += v2.tangent.x;
    v3.pos /= 2;
    v3.tex /= 2;
    Multiply(v3.color, 0.5f, v3.color);
    tangent.y += v2.tangent.y;
    tangent.z += v2.tangent.z;
    Normalize(v3.norm, v3.norm);
    Normalize(tangent, tangent);
    v3.tangent.x = tangent.x;
    v3.tangent.y = tangent.y;
    v3.tangent.z = tangent.z;
    v3.color.Zero();
}

void RndAmbientOcclusion::BurnTransform(
    RndMesh *mesh, std::list<RndMesh *> &meshes
) const {
    auto it = std::find(meshes.begin(), meshes.end(), mesh);
    if (it != meshes.end()) {
        meshes.erase(it);
        bool b5 = fabsf(1 - Det(mesh->LocalXfm().m)) > 0.0001f;
        if (mQuality == kQuality_Accurate) {
            b5 = CanBurnXfm(mesh);
        } else if (b5) {
            MILO_NOTIFY_ONCE(
                "%s: Mesh has scale or mirroring applied. Re-export mesh to ensure accurate AO calculation.",
                PathName(mesh)
            );
            b5 = false;
        }
        if (b5) {
            FOREACH (it, mesh->Children()) {
                RndMesh *cur = dynamic_cast<RndMesh *>(*it);
                if (cur) {
                    BurnTransform(cur, meshes);
                    Transform tfe0(Hmx::Matrix3(mesh->LocalXfm().m), Vector3(0, 0, 0));
                    Transform tfa0;
                    if (cur->TransConstraint() == RndTransformable::kConstraintNone) {
                        Multiply(cur->LocalXfm(), tfe0, tfa0);
                    } else if (
                        cur->TransConstraint() == RndTransformable::kConstraintParentWorld
                    ) {
                        tfa0 = tfe0;
                        cur->SetTransConstraint(
                            RndTransformable::kConstraintNone,
                            cur->GetTarget(),
                            cur->PreserveScale()
                        );
                    } else {
                        tfa0 = cur->LocalXfm();
                    }
                    cur->SetLocalXfm(tfa0);
                }
            }
            BurnXfm(mesh, true);
        }
    }
}

void RndAmbientOcclusion::CalculateAOAtPoint(
    const Vector3 &v1, const Vector3 &v2, float *fptr
) const {
    float f16 = gUnitsPerMeter * 50;
    Vector3 vb0;
    ScaleAdd(v1, v2, 0.001, vb0);
    int numVectors = unkb8.size();
    float f14 = 1 / f16;
    double f90[4] = { 0, 0, 0, 0 };
    for (int i = 0; i != numVectors; i++) {
        const Vector3 &curVec = unkb8[i];
        float dot = Dot(v2, curVec);
        if (dot > 0) {
            float fref;
            float f15 = 1;
            if (mTree->Intersect(vb0, curVec, f16, fref) && fref <= f16) {
                fref *= f14;
                f15 = fref * fref;
            }
            float fa0[4];
            BuildSHCoeff(curVec, fa0);
            for (int j = 0; j < 4; j++) {
                f90[j] += fa0[j] * f15 * dot;
            }
        }
    }
    for (unsigned int i = 0; i < 4; i++) {
        f90[i] *= 12.566371f / numVectors;
        if (i == 0) {
            f90[i] = Clamp<float>(0.0f, 1.0f, f90[i]);
        } else {
            f90[i] = (Clamp<float>(-1.0f, 1.0f, f90[i]) + 1) * 0.5;
        }
    }
    fptr[0] = f90[0];
    fptr[1] = f90[1];
    fptr[2] = f90[2];
    fptr[3] = f90[3];
}

void RndAmbientOcclusion::CalculateAO(float *fptr) {
    if (!mObjectsReceive.empty() && mTree) {
        unsigned int i5 = 0;
        FOREACH (it, mObjectsReceive) {
            RndMesh *mesh = *it;
            if (mesh->GetGeomOwner() != mesh) {
                mesh->CopyGeometry(mesh->GetGeomOwner(), true);
                mesh->Sync(0x3F);
            }
            i5 += mesh->Verts().size();
        }
        MILO_LOG("RndAmbientOcclusion: Calculating ambient occlusion...\n");
        Timer timer;
        timer.Restart();
        PreprocessMesh();
        unsigned int i7 = 0;
        unsigned int i6 = 0;
        FOREACH (it, mObjectsReceive) {
            RndMesh *mesh = *it;
            const Transform &world = mesh->WorldXfm();
            unsigned int i9 = i7 * 100;
            for (unsigned int i = 0; i < mesh->Verts().size(); i++, i7++, i9 += 100) {
                auto &curVert = mesh->Verts(i);
                Vector3 v10e0;
                Multiply(curVert.pos, world, v10e0);
                Vector3 v10d0;
                TransformNormal(curVert.norm, world.m, v10d0);
                CalculateAOAtPoint(v10e0, v10d0, (float *)&curVert.color);
                unsigned int i4 = i9 / i5;
                if (i4 != i6) {
                    i6 = i4;
                }
            }
            SmoothResults(mesh);
            mesh->SetHasAOCalc(true);
        }
        float secs = timer.SplitMs() / 1000;
        MILO_LOG("RndAmbientOcclusion: AO calculation took %0.2f seconds\n", secs);
        if (fptr) {
            *fptr = secs;
        }
        timer.Restart();
        FOREACH (it, mObjectsReceive) {
            (*it)->Sync(0x1F);
        }
    }
}
