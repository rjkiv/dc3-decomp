#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include <vector>
#include <list>

class CharClip;

class ByteQuat {
public:
    void Set(const Hmx::Quat &);
    void ToQuat(Hmx::Quat &q) const {
        q.Set(x * 0.007874016f, y * 0.007874016f, z * 0.007874016f, w * 0.007874016f);
    }

    char x;
    char y;
    char z;
    char w;
};

class ShortQuat {
public:
    void Set(const Hmx::Quat &);
    void ToQuat(Hmx::Quat &q) const {
        q.Set(
            x * 0.000030518509f,
            y * 0.000030518509f,
            z * 0.000030518509f,
            w * 0.000030518509f
        );
    }

    short x;
    short y;
    short z;
    short w;
};

class ShortVector3 {
public:
    void Set(const Vector3 &);

    void ToVector3(Vector3 &v) const {
        v.Set(x * 0.039674062f, y * 0.039674062f, z * 0.039674062f);
    }

    static short ToShort(float f) {
        float value = f * 0.00076923077f;
        value *= 32767.0f;
        value += 0.5f;
        return floor(Clamp(-32767.0f, 32767.0f, value));
    }

    short x;
    short y;
    short z;
};

inline short MakeShortAng(float f) {
    f = f * 1638.4f + 0.5f;
    MILO_ASSERT(f < 32768 && f > -32767, 0x60);
    return floor(f);
}

class CharBones {
public:
    enum Type {
        TYPE_POS = 0,
        TYPE_SCALE = 1,
        TYPE_QUAT = 2,
        TYPE_ROTX = 3,
        TYPE_ROTY = 4,
        TYPE_ROTZ = 5,
        TYPE_END = 6,
        NUM_TYPES = 7,
    };

    enum CompressionType {
        kCompressNone,
        kCompressRots,
        kCompressVects,
        kCompressQuats,
        kCompressAll
    };

    struct Bone {
        Bone() : name(), weight(1.0f) {}
        Bone(Symbol s, float w) : name(s), weight(w) {}
        Bone(const Bone &bone) : name(bone.name), weight(bone.weight) {}

        /** "Bone to blend into" */
        Symbol name;
        /** "Weight to blend with" */
        float weight;
    };

    CharBones();
    virtual ~CharBones() {}
    virtual void ScaleAdd(CharClip *, float, float, float);
    virtual void Print();

    void Zero();
    int TypeSize(int) const;
    void SetCompression(CompressionType);
    int FindOffset(Symbol) const;
    void *FindPtr(Symbol) const;
    const char *StringVal(Symbol);
    void SetWeights(float);
    void ListBones(std::list<Bone> &) const;
    void ClearBones();
    void AddBones(const std::vector<Bone> &);
    void AddBones(const std::list<Bone> &);
    void ScaleAdd(CharBones &, float) const;
    void ScaleDown(CharBones &, float) const;
    void RotateBy(CharBones &) const;
    void RotateTo(CharBones &, float) const;
    void Enter() {
        Zero();
        SetWeights(0);
    }
    void ScaleAddIdentity();
    void Blend(CharBones &) const;
    CompressionType GetCompression() const { return mCompression; }
    int TotalSize() const { return mTotalSize; }
    std::vector<Bone> GetBones() { return mBones; }
    Bone GetBonesAt(int index) { return mBones[index]; }
    Vector3 *VecOffset() const { return (Vector3 *)mStart; }
    Vector3 *ScaleOffset() const { return (Vector3 *)(mStart + mScaleOffset); }
    Hmx::Quat *QuatOffset() const { return (Hmx::Quat *)(mStart + mQuatOffset); }
    float *RotOffset() const { return (float *)(mStart + mRotXOffset); }
    float *RotYOffset() const { return (float *)(mStart + mRotYOffset); }
    float *RotZOffset() const { return (float *)(mStart + mRotZOffset); }
    char *EndOffset() const { return mStart + mEndOffset; }

    static Type TypeOf(Symbol);
    static const char *SuffixOf(Type);
    static Symbol ChannelName(const char *, Type);
    static void SetWeights(float, std::vector<Bone> &);

protected:
    virtual void ReallocateInternal() {}

    void RecomputeSizes();
    void AddBoneInternal(const Bone &);

    CompressionType mCompression; // 0x4
    /** "Bones that are animated" */
    std::vector<Bone> mBones; // 0x8
    char *mStart; // 0x14
    union {
        struct {
            int mPosCount; // 0x18
            int mScaleCount; // 0x1c
            int mQuatCount; // 0x20
            int mRotXCount; // 0x24
            int mRotYCount; // 0x28
            int mRotZCount; // 0x2c
            int mEndCount; // 0x30
        };
        int mCounts[NUM_TYPES]; // 0x18 - 0x30
    };
    struct {
        union {
            struct {
                int mPosOffset; // 0x34
                int mScaleOffset; // 0x38
                int mQuatOffset; // 0x3c
                int mRotXOffset; // 0x40
                int mRotYOffset; // 0x44
                int mRotZOffset; // 0x48
                int mEndOffset; // 0x4c
            };
            int mOffsets[NUM_TYPES]; // 0x34 - 0x4c
        };
        int mTotalSize; // 0x50
    };
};

/** "Holds state for a set of bones" */
class CharBonesObject : public CharBones, public virtual Hmx::Object {
public:
    OBJ_CLASSNAME(CharBonesObject);
    OBJ_SET_TYPE(CharBonesObject);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    NEW_OBJ(CharBonesObject)
    static void Init() { REGISTER_OBJ_FACTORY(CharBonesObject) }
};

/** "Holds state for a set of bones, and allocates own space" */
class CharBonesAlloc : public CharBonesObject {
public:
    virtual ~CharBonesAlloc();

    MEM_OVERLOAD(CharBonesAlloc, 0x172);

    friend class CharMirror;

protected:
    virtual void ReallocateInternal();
};

BinStream &operator<<(BinStream &, const CharBones::Bone &);
BinStream &operator>>(BinStream &, CharBones::Bone &);
bool PropSync(CharBones ::Bone &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);
extern CharBones *gPropBones;
