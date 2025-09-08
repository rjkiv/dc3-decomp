#pragma once
#include "char/CharBones.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"
#include "char/CharBonesSamples.h"

struct CharGraphNode {
    /** "where to blend from in my clip" */
    float curBeat;
    /** "where to blend to in clip" */
    float nextBeat;
};

/** "This is the simple form that stores
 *  samples and linearly interpolates between them.
 *  Data is grouped by keyframe, for better RAM coherency
 *  better storage, interpolation, etc." */
class CharClip : public Hmx::Object {
public:
    class NodeVector {
    public:
    };
    class Transitions : public ObjRefOwner {
    public:
        Transitions(Hmx::Object *owner)
            : mNodeStart(nullptr), mNodeEnd(nullptr), mOwner(owner) {}
        virtual ~Transitions();
        virtual Hmx::Object *RefOwner() const { return mOwner; }
        virtual bool Replace(ObjRef *, Hmx::Object *);

        NodeVector *mNodeStart; // 0x4
        NodeVector *mNodeEnd; // 0x8
        Hmx::Object *mOwner; // 0xc
    };

    class BeatEvent {
    public:
        BeatEvent();
        BeatEvent(const BeatEvent &);
        BeatEvent &operator=(const BeatEvent &);
        void Load(BinStream &);

        /** "The event argument for the {clip_event <event> <clip>}
            message exported to the controlling Character" */
        Symbol event;
        /** "Clip Beat the event should trigger" */
        float beat;
    };

    class FacingSet {
    public:
        class FacingBones : public CharBones {
        public:
            FacingBones();
            virtual ~FacingBones() {}
            virtual void ReallocateInternal();

            void Set(bool);

            Vector3 mDeltaPos; // 0x54
            float mDeltaAng; // 0x64
        };

        FacingSet() : mFullRot(-1), mFullPos(-1), mFacingBones(nullptr), mWeight(1) {}
        void ListBones(std::list<CharBones::Bone> &);
        void Set(CharBonesSamples &);
        void ScaleDown(CharBones &, float);
        void
        ScaleAddSample(CharBonesSamples &, CharBones &, float, int, float, int, float);

        static void Init();
        static FacingBones sFacingPos;
        static FacingBones sFacingRotAndPos;

        int mFullRot; // 0x0
        int mFullPos; // 0x4
        FacingBones *mFacingBones; // 0x8
        float mWeight; // 0xc
    };

    virtual ~CharClip();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CharClip);
    OBJ_SET_TYPE(CharClip);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PostSave(BinStream &) {}
    virtual void Print();
    virtual void SetTypeDef(DataArray *);

    NEW_OBJ(CharClip)
    static void Init();
    static void *operator new(unsigned int s) {
        static int _x = MemFindHeap("char");
        MemPushHeap(_x);
        void *mem = MemAlloc(s, __FILE__, 0x51, StaticClassName().Str(), 0);
        MemPopHeap();
        return mem;
    }
    static void *operator new(unsigned int s, void *place) { return place; }
    static void operator delete(void *v) {
        MemFree(v, __FILE__, 0x51, StaticClassName().Str());
    }

protected:
    CharClip();

    Transitions mTransitions; // 0x2c
    /** "Frames per second" */
    float mFramesPerSec; // 0x3c
    Keys<float, float> mBeatTrack; // 0x40
    /** "Search flags, app specific" */
    int mFlags; // 0x4c
    int mPlayFlags; // 0x50
    /** "Range in frames to randomly offset by when playing" */
    float mRange; // 0x54
    /** "Make the clip all relative to this other clip's first frame" */
    ObjPtr<CharClip> mRelative; // 0x58
    /** "Events that get triggered during clip playback,
        exports {clip_event <event> <clip>} to the character owner,
        you get enter and exit events for free" */
    std::vector<BeatEvent> mBeatEvents; // 0x6c
    /** "Indicates transition graph needs updating" */
    bool mDirty; // 0x78
    int mOldVer; // 0x7c
    /** "Check this to prevent any compression from happening on this clip" */
    bool mDoNotCompress; // 0x80
    /** "An animatable, like a PropAnim, you'd like play in sync with this clip" */
    ObjPtr<RndAnimatable> mSyncAnim; // 0x84
    CharBonesSamples mFull; // 0x98
    CharBonesSamples mOne; // 0x104
    FacingSet mFacing; // 0x170
    std::vector<CharBones::Bone> mZeros; // 0x180
    std::vector<std::map<int, float> > unk18c; // 0x18c
    int unk198; // 0x198
};
