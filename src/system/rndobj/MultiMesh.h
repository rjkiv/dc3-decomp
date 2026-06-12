#pragma once
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

struct OldMMInst {
    Transform mOldXfm; // 0x0
    Hmx::Color mOldColor; // 0x30
};

inline BinStream &operator>>(BinStream &bs, OldMMInst &inst) {
    bs >> inst.mOldXfm >> inst.mOldColor;
    return bs;
}

extern ReclaimableAlloc gTransListAlloc;

#ifdef STLPORT
// TransformListAlloc exists in the STLport namespace
namespace STLPORT {
#endif

    template <class T>
    class TransformListAlloc {
    public:
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef const T *const_pointer;
        typedef const T &const_reference;

        template <class T2>
        struct rebind {
            typedef TransformListAlloc<T2> other;
        };

        // #ifdef VERSION_SZBE69_B8
        // Retail doesn't have constructor calls
        TransformListAlloc() {}
        TransformListAlloc(TransformListAlloc<T> const &) {}
        template <class T2>
        TransformListAlloc(const TransformListAlloc<T2> &) {}
        // #endif

        // ...but still has the destructor
        ~TransformListAlloc() {}

        pointer allocate(const size_type count, const void *hint = nullptr) const {
            return reinterpret_cast<pointer>(gTransListAlloc.CustAlloc(count * sizeof(T)));
        }

        void deallocate(pointer ptr, size_type count) const {
            gTransListAlloc.CustFree(ptr);
        }
    };

#ifdef STLPORT
}
#endif

/**
 * @brief An object that instances \ref RndMesh "RndMeshes".
 * Original _objects description:
 * "A MultiMesh object draws a simple Mesh in many places
 * quickly. Create a multimesh by instancing the base mesh lots of
 * times in Max, then using the multiobject wizard."
 */
class RndMultiMesh : public RndDrawable {
public:
    struct Instance {
        Instance();
        Instance(const Transform &t) : unk0(1), mXfm(t) {}
        Instance &operator=(const Instance &other) {
            memcpy(this, &other, sizeof(*this));
            return *this;
        }

        void Save(BinStream &) const;
        void Load(BinStreamRev &);
        void LoadRev(BinStream &, int);

        bool unk0; // 0x0
        Transform mXfm; // 0x4
    };

    typedef std::
        list<RndMultiMesh::Instance, std::TransformListAlloc<RndMultiMesh::Instance> >
            InstanceList;

    // Hmx::Object
    virtual ~RndMultiMesh() { InvalidateProxies(); }
    OBJ_CLASSNAME(MultiMesh);
    OBJ_SET_TYPE(MultiMesh);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndDrawable
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual void CollideList(const Segment &, std::list<Collision> &);

    OBJ_MEM_OVERLOAD(0x1C);
    NEW_OBJ(RndMultiMesh)
    static void Init() { REGISTER_OBJ_FACTORY(RndMultiMesh) }
    static void Terminate();
    static const std::list<std::pair<class RndMultiMeshProxy *, int> > &ProxyPool() {
        return sProxyPool;
    }

    RndMesh *Mesh() const { return mMesh; }
    void SetMesh(RndMesh *);
    InstanceList &Instances() { return mInstances; }
    Instance &Instances(int idx) { return *NextItr(mInstances.begin(), idx); }
    void RemoveInstance(int idx) { mInstances.erase(NextItr(mInstances.begin(), idx)); }
    void InvalidateProxies();

protected:
    RndMultiMesh();

    virtual void UpdateMesh() {}

    DataNode OnMoveXfms(const DataArray *);
    DataNode OnScaleXfms(const DataArray *);
    DataNode OnSortXfms(const DataArray *);
    DataNode OnRandomXfms(const DataArray *);
    DataNode OnScrambleXfms(const DataArray *);
    DataNode OnDistribute(const DataArray *);
    DataNode OnGetPos(const DataArray *);
    DataNode OnSetPos(const DataArray *);
    DataNode OnGetRot(const DataArray *);
    DataNode OnSetRot(const DataArray *);
    DataNode OnGetScale(const DataArray *);
    DataNode OnSetScale(const DataArray *);
    DataNode OnMesh(const DataArray *);
    DataNode OnAddXfm(const DataArray *);
    DataNode OnAddXfms(const DataArray *);
    DataNode OnRemoveXfm(const DataArray *);
    DataNode OnNumXfms(const DataArray *);

    static std::list<std::pair<class RndMultiMeshProxy *, int> > sProxyPool;

    /** The simple mesh to draw. */
    ObjPtr<RndMesh> mMesh; // 0x20
    /** The locations at which the mesh should be drawn. */
    InstanceList mInstances; // 0x2C
};

typedef std::list<RndMultiMesh::Instance, std::TransformListAlloc<RndMultiMesh::Instance> >
    InstanceList;

inline BinStreamRev &operator>>(BinStreamRev &d, RndMultiMesh::Instance &inst) {
    inst.Load(d);
    return d;
}
