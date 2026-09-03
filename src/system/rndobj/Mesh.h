#pragma once
#include "math/Color.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

class RndMultiMesh;

class MotionBlurCache {
public:
    MotionBlurCache() {
        mCacheKey[0] = 0;
        mCacheKey[1] = 0;
        mShouldCache = false;
    }
    unsigned int mCacheKey[2]; // 0x0, 0x4
    bool mShouldCache; // 0x8
};

/** A bone to associate with a Mesh. */
class RndBone {
public:
    RndBone(Hmx::Object *o) : mBone(o) {}
    void Load(BinStream &);

    /** "Trans of the bone" */
    ObjPtr<RndTransformable> mBone; // 0x0
    Transform mOffset;
};

/**
 * @brief A mesh object, used to make models.
 * Original _objects description:
 * "A Mesh object is composed of triangle faces."
 */
class RndMesh : public RndDrawable, public RndTransformable {
public:
    enum Volume {
        kVolumeEmpty,
        kVolumeTriangles,
        kVolumeBSP,
        kVolumeBox
    };

    class Vert {
    public:
        Vert()
            : pos(0, 0, 0), norm(0, 1, 0), boneWeights(0, 0, 0, 0), color(1, 1, 1, 1),
              tex(0, 0) {
            boneIndices[0] = 0;
            boneIndices[1] = 1;
            boneIndices[2] = 2;
            boneIndices[3] = 3;
            tangent.Set(1, 0, 0, 1);
        }

        static void *operator new(unsigned int s) {
            return _MemAllocTemp(s, __FILE__, 0x78, "Vert", 0);
        }
        static void *operator new(unsigned int s, void *place) { return place; }
        static void *operator new[](unsigned int s) {
            return _MemAllocTemp(s, __FILE__, 0x78, "Vert", 0);
        }
        static void operator delete(void *v) { MemFree(v, __FILE__, 0x78, "Vert"); }
        static void operator delete[](void *v) { MemFree(v, __FILE__, 0x78, "Vert"); }

        Vector3 pos; // 0x0
        Vector3 norm; // 0x10
        Vector4 boneWeights; // 0x20
        Hmx::Color color; // 0x30
        Vector2 tex; // 0x40
        short boneIndices[4]; // 0x48
        Vector4 tangent; // 0x50
    };

    /** A triangle mesh face. */
    class Face {
    public:
        Face(int inv1 = 0, int inv2 = 0, int inv3 = 0) : v1(inv1), v2(inv2), v3(inv3) {}
        unsigned short &operator[](int i) { return *(&v1 + i); }
        void Set(int i0, int i1, int i2) {
            v1 = i0;
            v2 = i1;
            v3 = i2;
        }

        /** The three points that make up the face. */
        unsigned short v1, v2, v3;
    };

    /** A specialized vector for RndMesh vertices. */
    class VertVector { // more custom STL! woohoo!!!! i crave death
        friend bool PropSync(
            RndMesh ::VertVector &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op
        );

    public:
        VertVector() : mVerts(nullptr), mNumVerts(0), mCapacity(0), unkc(0) {}
        ~VertVector() {
            mCapacity = 0;
            clear();
        }
        int size() const { return mNumVerts; }
        bool empty() const { return mNumVerts == 0; }
        Vert &operator[](int i) { return mVerts[i]; }
        const Vert &operator[](int i) const { return mVerts[i]; }
        void clear() { resize(0); }
        void resize(int);
        Vert *begin() { return &mVerts[0]; }
        Vert *end() { return &mVerts[mNumVerts]; }
        void operator=(const VertVector &);

        Vert *mVerts; // 0x0
        int mNumVerts; // 0x4
        int mCapacity; // 0x8
        bool unkc;
    };

    virtual ~RndMesh();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Mesh);
    OBJ_SET_TYPE(Mesh);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();

    virtual void UpdateSphere();
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void Highlight() { RndDrawable::Highlight(); }
    virtual void LoadVertices(BinStreamRev &);
    virtual void SaveVertices(BinStream &);
    virtual void DrawFacesInRange(int, int) {}
    /** "Number of faces in the mesh" */
    virtual int NumFaces() const { return mFaces.size(); }
    /** "Number of verts in the mesh" */
    virtual int NumVerts() const { return mVerts.size(); }

    OBJ_MEM_OVERLOAD(0x2E);
    NEW_OBJ(RndMesh)
    static void Init() { REGISTER_OBJ_FACTORY(RndMesh) }

    int EstimatedSizeKb() const;
    void SetMat(RndMat *);
    void SetGeomOwner(RndMesh *);
    void SetKeepMeshData(bool);
    void SetNumBones(int);
    void SetBone(int, RndTransformable *, bool);
    VertVector &Verts() { return mGeomOwner->mVerts; }
    std::vector<Face> &Faces() { return mGeomOwner->mFaces; }
    Vert &Verts(int idx) { return mGeomOwner->mVerts[idx]; }
    Face &Faces(int idx) { return mGeomOwner->mFaces[idx]; }
    Volume GetVolume() const { return mGeomOwner->mVolume; }
    BSPNode *GetBSPTree() const { return mGeomOwner->mBSPTree; }
    bool GetKeepMeshData() const { return mKeepMeshData; }
    RndMat *Mat() const { return mMat; }
    bool IsSkinned() const { return !mBones.empty(); }
    int MaxBones() const { return GetGfxMode() != kOldGfx ? 40 : 4; }
    int NumBones() const { return mBones.size(); }
    void SetMutable(int m) { mGeomOwner->mMutable = m; }
    int Mutable() const { return mGeomOwner->mMutable; }
    bool HasAOCalc() const { return mGeomOwner->mHasAOCalc; }
    void SetHasAOCalc(bool calc) { mGeomOwner->mHasAOCalc = calc; }
    RndMesh *GetGeomOwner() const { return mGeomOwner; }
    MotionBlurCache &GetBlurCache() { return mMotionCache; }
    RndTransformable *BoneTransAt(int idx) { return mBones[idx].mBone; }
    Transform &BoneOffsetAt(int idx) { return mBones[idx].mOffset; }
    void InstanceGeomOwnerBones();
    void DeleteBones(bool);
    void BurnXfm();
    void ResetNormals();
    void Tessellate();
    void ClearAO();
    void CopyGeometryFromOwner();
    void Sync(int);
    void SetVolume(Volume);
    void CopyBones(const RndMesh *);
    void CopyGeometry(const RndMesh *, bool);
    void SetZeroWeightBones();
    int CollidePlane(const RndMesh::Face &, const Plane &);
    Vector3 SkinVertex(const RndMesh::Vert &, Vector3 *);
    void ScaleBones(float);
    int GetBoneIndex(const RndTransformable *);
    RndMultiMesh *CreateMultiMesh();

protected:
    RndMesh();

    virtual void OnSync(int flags);

    void ClearCompressedVerts();
    bool PatchOkay(int i, int j) { return i * 4.31 + j * 0.25 < 329.0; }
    bool HasInstancedBones();
    bool HasValidBones(unsigned int *) const;
    void SetNumVerts(int verts);
    void SetNumFaces(int faces);
    void RemoveInvalidBones();

    DataNode OnCompareEdgeVerts(const DataArray *);
    DataNode OnAttachMesh(const DataArray *);
    DataNode OnGetFace(const DataArray *);
    DataNode OnSetFace(const DataArray *);
    DataNode OnGetVertXYZ(const DataArray *);
    DataNode OnSetVertXYZ(const DataArray *);
    DataNode OnGetVertNorm(const DataArray *);
    DataNode OnSetVertNorm(const DataArray *);
    DataNode OnGetVertUV(const DataArray *);
    DataNode OnSetVertUV(const DataArray *);
    DataNode OnUnitizeNormals(const DataArray *);
    DataNode OnBuildFromBSP(const DataArray *);
    DataNode OnPointCollide(const DataArray *);
    DataNode OnConfigureMesh(const DataArray *);

    static bool sRawCollide;
    static int sLastCollide;

    /** This mesh's vertices. */
    VertVector mVerts; // 0x100
    /** This mesh's faces. */
    std::vector<Face> mFaces; // 0x110
    /** "Material used for rendering the Mesh" */
    ObjPtr<RndMat> mMat; // 0x11c
    std::vector<unsigned char> mPatches; // 0x130
    /** "Geometry owner for the mesh" */
    ObjOwnerPtr<RndMesh> mGeomOwner; // 0x13c
    /** This mesh's bones. */
    ObjVector<RndBone> mBones; // 0x150
    int mMutable; // 0x160
    /** "Volume of the Mesh" */
    Volume mVolume; // 0x164
    BSPNode *mBSPTree; // 0x168
    /** The MultiMesh that will draw this Mesh multiple times. */
    RndMultiMesh *mMultiMesh; // 0x16c
    bool mHasAOCalc; // 0x170
    bool mKeepMeshData; // 0x171
    MotionBlurCache mMotionCache; // 0x174
    int unk180;
    unsigned char *mCompressedVerts; // 0x184
    unsigned int mNumCompressedVerts; // 0x188
};

class PatchVerts {
public:
    PatchVerts() : mCentroid(0, 0, 0) {}
    ~PatchVerts() {}

    int NumVerts() const { return mPatchVerts.size(); }

    void Add(int vert, RndMesh::VertVector &verts, Vector3 &centroid);

    void Clear() {
        mPatchVerts.clear();
        mCentroid.Set(0, 0, 0);
    }

    bool HasVert(int vert) const {
        int idx = GreaterEq(vert);
        if (idx < mPatchVerts.size()) {
            return mPatchVerts[idx] == vert;
        } else
            return false;
    }

protected:
    int GreaterEq(int iii) const {
        if (!mPatchVerts.empty() && iii > mPatchVerts.front()) {
            if (iii > mPatchVerts.back()) {
                return mPatchVerts.size();
            } else {
                int u5 = 0;
                int u2 = mPatchVerts.size() - 1;
                while (u2 > u5 + 1) {
                    int u4 = (u5 + u2) >> 1;
                    int curVert = mPatchVerts[u4];
                    if (iii > curVert) {
                        u5 = u4;
                    } else {
                        u2 = u4;
                    }
                }
                return u2;
            }
        } else
            return 0;
    }

    Vector3 mCentroid; // 0x0
    std::vector<int> mPatchVerts; // 0x10
};
