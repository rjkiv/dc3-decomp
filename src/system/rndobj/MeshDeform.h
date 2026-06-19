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
        VertArray(RndMeshDeform *md) : mSize(0), mData(nullptr), mParent(md) {}
        ~VertArray() { MemFree(mData); }
        void Clear() { SetSize(0); }
        void *FindVert(int);
        void CopyVert(int, int, VertArray &);
        int AppendWeights(int num, int *const bones, float *const weights);
        void Copy(const VertArray &);
        void Save(BinStream &);
        void Load(BinStream &);
        int NumVerts() {
            int num = 0;
            auto itEnd = end();
            for (auto it = begin(); it < itEnd; ++it) {
                num++;
            }
            return num;
        }

        struct WeightPair {
            unsigned char bone; // 0x0
            unsigned char weight; // 0x1
        };

        struct Vert {
            unsigned char num; // 0x0
            WeightPair weights[kMaxWeights]; // 0x1
        };

        class iterator {
        private:
            unsigned char *data;

        public:
            iterator() : data(nullptr) {}
            iterator(unsigned char *d) : data(d) {}
            operator unsigned char *() const { return data; }
            unsigned char *&operator*() { return data; }

            iterator &operator++() {
                // skips to the next Vert over,
                // based on the number of WeightPairs in this current Vert
                Vert *cur = (Vert *)data;
                data += (cur->num * sizeof(WeightPair)) + 1;
                return *this;
            }

            bool operator!=(const iterator &it) { return data != it.data; }
            bool operator==(const iterator &it) { return data == it.data; }
        };

        iterator begin() const { return mData; }
        iterator end() const { return mData + mSize; }

    protected:
        void SetSize(int);

        int mSize; // 0x0
        // mData is a tightly packed series of Verts
        // the reason this is unsigned char* and not Vert* is
        // so it doesn't take up the full kMaxWeights sized array in memory.
        // it only takes up however many weights the Vert has,
        // and you access it via a reinterpret_cast to a Vert
        unsigned char *mData; // 0x4
        RndMeshDeform *mParent; // 0x8
    };

    // size 0x6c
    struct BoneDesc {
        BoneDesc(Hmx::Object *owner) : unk0(owner) {
            unk14.Reset();
            unk54.Reset();
        }

        ObjPtr<RndTransformable> unk0;
        Transform unk14;
        Transform unk54;
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
    static void Init() { REGISTER_OBJ_FACTORY(RndMeshDeform) }

protected:
    RndMeshDeform();

    /** "The mesh we will change, set you can make a zero vert meshdeform
        just to clean up mutable character meshes" */
    ObjPtr<RndMesh> mMesh; // 0x2c
    Transform mMeshInverse; // 0x40
    ObjVector<BoneDesc> mBones; // 0x80
    VertArray mVerts; // 0x94
    bool mSkipInverse;
    bool mDeformed;
};
