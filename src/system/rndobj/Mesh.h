#pragma once
#include "math/Color.h"
#include "obj/Object.h"
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
        Vector3 pos; // 0x0
        Vector3 norm; // 0x10
        Vector4 boneWeights; // 0x20
        Hmx::Color color; // 0x30
        Vector2 tex; // 0x40
        short boneIndices[4]; // 0x48
        Vector4 unk50; // 0x50
    };

    /** A triangle mesh face. */
    class Face {
    public:
        Face() : v1(0), v2(0), v3(0) {}
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

        Vert *mVerts; // 0x0
        int mNumVerts; // 0x4
        int mCapacity; // 0x8
        bool unkc;
    };

    virtual ~RndMesh() {}
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
    virtual int NumFaces() const;
    /** "Number of verts in the mesh" */
    virtual int NumVerts() const;
    virtual void OnSync(int);

    OBJ_MEM_OVERLOAD(0x2E);

    int EstimatedSizeKb() const;
    void SetMat(RndMat *);
    void SetGeomOwner(RndMesh *);
    void SetKeepMeshData(bool);
    void SetNumBones(int);
    void SetBone(int, RndTransformable *, bool);
    void SetNumVerts(int verts) { mGeomOwner->mVerts.resize(verts); }
    void SetNumFaces(int faces) { mGeomOwner->mFaces.resize(faces); }

protected:
    RndMesh();

    void ClearCompressedVerts();
    bool PatchOkay(int i, int j) { return i * 4.31 + j * 0.25 < 329.0; }

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
    class BSPNode *mBSPTree; // 0x168
    /** The MultiMesh that will draw this Mesh multiple times. */
    RndMultiMesh *mMultiMesh; // 0x16c
    bool unk170;
    bool mKeepMeshData; // 0x171
    MotionBlurCache mMotionCache; // 0x174
    int unk180;
    unsigned char *mCompressedVerts; // 0x184
    unsigned int mNumCompressedVerts; // 0x188
};
