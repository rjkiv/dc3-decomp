#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

/** "Reskins target mesh according to exobones." */
class RndMeshDeform : public Hmx::Object {
public:
    class VertArray {
    public:
        enum {
            kMaxWeights = 0x40
        };
        struct Vert {
            struct WeightPair {
                unsigned char bone; // 0x0
                unsigned char weight; // 0x1

                float GetWeight() const { return weight / 255.0f; }
                void SetWeight(float wt) { weight = wt * 255.0f + 0.5f; }
            };

            Vert *Next() const {
                // skips to the next Vert over,
                // based on the number of WeightPairs in this current Vert
                return (Vert *)((unsigned char *)this + num * sizeof(WeightPair) + 1);
            }

            int Size() const;

            unsigned char num; // 0x0
            WeightPair weights[kMaxWeights]; // 0x1
        };

        VertArray(RndMeshDeform *md) : mSize(0), mData(nullptr), mParent(md) {}
        ~VertArray() { MemFree(mData); }
        void Clear() { SetSize(0); }
        void CopyVert(int, int, VertArray &);
        int AppendWeights(int num, int *const bones, float *const weights);
        void Copy(const VertArray &);
        void Save(BinStream &);
        void Load(BinStream &);
        int NumVerts() {
            int num = 0;
            for (Vert *it = Begin(); it < End(); it = it->Next()) {
                num++;
            }
            return num;
        }
        Vert *Begin() { return (Vert *)mData; }
        Vert *End() { return (Vert *)((unsigned char *)mData + mSize); }
        int Size() const { return mSize; }

        // public: void ShrinkVerts(int32_t);

    protected:
        void SetSize(int);
        Vert *FindVert(int);

        int mSize; // 0x0
        // mData is a tightly packed series of Verts
        // the reason this is void* and not Vert* is
        // so it doesn't take up the full kMaxWeights sized array in memory.
        // it only takes up however many weights the Vert has,
        // and you access it via a reinterpret_cast to a Vert
        void *mData; // 0x4
        RndMeshDeform *mParent; // 0x8
    };

    // size 0x6c
    struct BoneDesc {
        BoneDesc(Hmx::Object *owner) : bone(owner) {
            offset.Reset();
            parent.Reset();
        }
        // void ExportWorldXfm(Transform&);

        ObjPtr<RndTransformable> bone; // 0x0
        Transform offset; // 0x14
        Transform parent; // 0x54
    };

    virtual ~RndMeshDeform();
    OBJ_CLASSNAME(MeshDeform);
    OBJ_SET_TYPE(MeshDeform);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void Print();

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(RndMeshDeform)
    REGISTER_OBJ(RndMeshDeform)

protected:
    RndMeshDeform();

    /** "The mesh we will change, set you can make a zero vert meshdeform
        just to clean up mutable character meshes" */
    ObjPtr<RndMesh> mMesh; // 0x2c
    Transform mMeshInverse; // 0x40
    ObjVector<BoneDesc> mBones; // 0x80
    VertArray mVerts; // 0x90
    bool mSkipInverse; // 0x9c
    bool mDeformed; // 0x9d
};
