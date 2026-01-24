#include "rndobj/AmbientOcclusion.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
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
unsigned int GatherObjectsFromGroup(RndGroup *, std::vector<T *> &objects);

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

void RndAmbientOcclusion::Load(BinStream &bs) {
    int revs;
    bs >> revs;
    BinStreamRev d(bs, revs);
    static const unsigned short gRevs[4] = { 4, 0, 0, 0 };
    if (d.rev > 4) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d",
            PathName(this),
            ClassName(),
            d.rev,
            gRevs[0]
        );
    }
    if (d.altRev > 0) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d",
            PathName(this),
            ClassName(),
            d.altRev,
            gRevs[2]
        );
    }
    Hmx::Object::Load(d.stream);
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
}

void RndAmbientOcclusion::BuildTrees(Quality quality) {
    MILO_ASSERT(quality < kQuality_Max, 0x1E3);
    mQuality = quality;
    if (!mObjectsCast.empty() && !mObjectsReceive.empty()) {
        MILO_ASSERT(mTriList.empty(), 0x1E9);
        Timer timer;
        timer.Restart();
        MILO_LOG("RndAmbientOcclusion: Building kd-Tree...\n");
        Box box(Vector3(FLT_MAX, FLT_MAX, FLT_MAX), Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
        FOREACH (it, mObjectsCast) {
        }
        MILO_ASSERT(mTree == NULL, 0x234);
        box.Extend(0.001f);
        mTree = new kdTree<Triangle>(box);
        FOREACH (it, mTriList) {
            mTree->Add(&*it);
        }
        // kdtree pack
        mTree->PackNodes((kdTree<Triangle>::SplitPlaneType)0, 0);
        MILO_LOG(
            "RndAmbientOcclusion: Built kd-Tree in %0.2f seconds\n",
            timer.SplitMs() / 1000.0f
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

void RndAmbientOcclusion::BuildSHCoeff(const Vector3 &inVector, float *fArr) const {
    MILO_ASSERT(Abs(1.0f - Length(inVector)) <= kSmallFloat, 0x298);
    fArr[0] = 0.2820948f;
    fArr[1] = inVector.y * 0.48860252f;
    fArr[2] = inVector.z * 0.48860252f;
    fArr[3] = inVector.x * 0.48860252f;
}

template <class T>
struct VectorSort {
    VectorSort(const std::vector<T> &v) : vector(v) {}

    bool operator()(T item1, T item2);

    const std::vector<T> &vector; // idk
};

void RndAmbientOcclusion::BuildObjectLists() {
    ObjectDir *myDir = Dir();
    Clean();
    MILO_ASSERT(mObjectsCast.empty(), 0x199);
    MILO_ASSERT(mObjectsReceive.empty(), 0x19A);
    MILO_ASSERT(mObjectsTessellate.empty(), 0x19B);
    std::vector<RndMesh *> meshes;
    GatherObjectsFromDir(myDir, meshes);
    std::unique_copy(meshes.begin(), meshes.end(), meshes.begin());
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
    std::unique_copy(dontCastMeshes.begin(), dontCastMeshes.end(), meshes.end());
    std::unique_copy(dontReceiveMeshes.begin(), dontReceiveMeshes.end(), meshes.end());
    std::unique_copy(tessellateMeshes.begin(), tessellateMeshes.end(), meshes.end());
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
        VectorSort<RndMesh *>(mObjectsTessellate)
    );
}

void RndAmbientOcclusion::TransformNormal(
    const Vector3 &vin, const Hmx::Matrix3 &min, Vector3 &vout
) const {
    Vector3 vtmp;
    Normalize(vin, vtmp);
    Hmx::Matrix3 mtmp;
    Invert(min, mtmp);
    // vout.x = vtmp.x * mtmp.x.x + vtmp.y * mtmp.x.y + vtmp.z * mtmp.x.z;
    // vout.y = vtmp.x * mtmp.y.x + vtmp.y * mtmp.y.y + vtmp.z * mtmp.y.z;
    // vout.z = vtmp.x * mtmp.z.x + vtmp.y * mtmp.z.y + vtmp.z * mtmp.z.z;
    vout.Set(Dot(vtmp, mtmp.x), Dot(vtmp, mtmp.y), Dot(vtmp, mtmp.z));
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
    ObjectDir *meshDir = mesh->Dir();
    return (meshDir == Dir())
        || (meshDir->IsSubDir() && meshDir->InlineSubDirType() == kInlineAlways);
}

bool RndAmbientOcclusion::IsValid_Mesh(const RndMesh *mesh) const {
    RndMesh *nonConstMesh = (RndMesh *)mesh; // lmao
    if (nonConstMesh->Verts().size() && nonConstMesh->Faces().size()) {
        static Symbol classNames[] = { "Spotlight", "WorldCrowd" };
        FOREACH (it, mesh->Refs()) {
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
    FOREACH (it, mesh->Refs()) {
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
