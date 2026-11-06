#pragma once
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include <vector>
#include <list>

class CharClip;

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
        kCompressNone = 0,
        kCompressRots = 1,
        kCompressVects = 2,
        kCompressQuats = 3,
        kCompressAll = 4
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
    virtual void ScaleAdd(CharClip *clip, float weight, float frame, float dframe);
    virtual void Print();

    int TypeSize(int type) const;
    int FindOffset(Symbol name) const;
    // int FindIndex(Symbol) const;
    void *FindPtr(Symbol channel) const;
    void SetWeights(float w);
    void ListBones(std::list<Bone> &bones) const;
    void ClearBones();
    void AddBones(const std::vector<Bone> &bones);
    void AddBones(const std::list<Bone> &bones);
    void ScaleAdd(CharBones &bones, float weight) const;
    void ScaleDown(CharBones &bones, float scale) const;
    void RotateBy(CharBones &bones) const;
    void RotateTo(CharBones &bones, float weight) const;
    void Zero();
    void ZeroOut() {
        Zero();
        SetWeights(0);
    }
    void ScaleAddIdentity();
    void Blend(CharBones &bones) const;

    //   public: bool BoneWeights();
    std::vector<Bone> &Bones() { return mBones; }
    const std::vector<Bone> &Bones() const { return mBones; }

    Vector3 *PosPtr(int idx) { return GetPos() + idx; }
    ShortVector3 *ShortPosPtr(int idx) { return GetShortPos() + idx; }
    Hmx::Quat *QuatPtr(int idx) { return GetQuat() + idx; }
    float *RotPtr(int idx) { return GetRotX() + idx; }
    Vector3 *ScalePtr(int idx) { return GetScale() + idx; }

    char *GetStart() { return mStart; }
    Vector3 *GetPos() const { return (Vector3 *)mStart; }
    ShortVector3 *GetShortPos() const { return (ShortVector3 *)mStart; }
    Vector3 *GetScale() const { return (Vector3 *)(mStart + mScaleOffset); }
    Hmx::Quat *GetQuat() const { return (Hmx::Quat *)(mStart + mQuatOffset); }
    ShortQuat *GetShortQuat() const { return (ShortQuat *)(mStart + mQuatOffset); }
    ByteQuat *GetByteQuat() const { return (ByteQuat *)(mStart + mQuatOffset); }
    float *GetRotX() const { return (float *)(mStart + mRotXOffset); }
    short *GetShortRotX() const { return (short *)(mStart + mRotXOffset); }
    float *GetRotY() const { return (float *)(mStart + mRotYOffset); }
    float *GetRotZ() const { return (float *)(mStart + mRotZOffset); }
    Vector3 *PosEnd() const { return (Vector3 *)(mStart + mScaleOffset); }
    Vector3 *ScaleEnd() const { return (Vector3 *)(mStart + mQuatOffset); }
    Hmx::Quat *QuatEnd() const { return (Hmx::Quat *)(mStart + mRotXOffset); }
    float *RotEnd() const { return (float *)(mStart + mEndOffset); }
    float *GetEnd() const { return (float *)(mStart + mEndOffset); }
    CompressionType Compression() const { return mCompression; }
    void SetCompression(CompressionType);
    int TotalSize() const { return mTotalSize; }
    const char *StringVal(Symbol);
    const int *Counts() const { return mCounts; }
    int *Counts() { return mCounts; }
    const int *Offsets() const { return mOffsets; }
    int *Offsets() { return mOffsets; }

    //   public: void SetPointers(char*);
    //   public: void SetFromMeshes(ObjectDir*);
    //   public: void CopyInto(CharBones&) const;
    //   public: DataNode ListChannels(CharBones::Type, CharBones::Type);

    static Type TypeOf(Symbol bone);
    static const char *SuffixOf(Type t);
    static Symbol ChannelName(const char *name, Type t);
    static void SetWeights(float w, std::vector<Bone> &s);

protected:
    virtual void ReallocateInternal() {}

    void RecomputeSizes();
    void AddBoneInternal(const Bone &bone);

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
bool PropSync(CharBones::Bone &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);
extern CharBones *gPropBones;
