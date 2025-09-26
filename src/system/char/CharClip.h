#pragma once
#include "char/CharBones.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/BinStream.h"
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
        NodeVector *Next() const { return (NodeVector *)(this->nodes + size); }

        ObjOwnerPtr<CharClip> clip; // 0x0
        int size; // 0x14
        CharGraphNode nodes[1]; // 0x18
    };
    class Transitions : public ObjRefOwner {
    public:
        Transitions(Hmx::Object *owner)
            : mNodeStart(nullptr), mNodeEnd(nullptr), mOwner(owner) {}
        virtual ~Transitions() { Clear(); }
        virtual Hmx::Object *RefOwner() const { return mOwner; }
        virtual bool Replace(ObjRef *, Hmx::Object *);

        void Clear();
        int Size() const;
        NodeVector *GetNodes(int) const;
        NodeVector *Resize(int, const NodeVector *);
        NodeVector *FindNodes(CharClip *) const;
        int BytesInMemory() const { return (int)mNodeEnd - (int)mNodeStart; }
        void RemoveNodes(NodeVector *);
        void Save(BinStream &);
        void Load(BinStreamRev &, int);
        void RemoveClip(CharClip *);

        NodeVector *mNodeStart; // 0x4
        NodeVector *mNodeEnd; // 0x8
        Hmx::Object *mOwner; // 0xc
    };

    class BeatEvent {
    public:
        BeatEvent() : beat(0) {}
        BeatEvent(const BeatEvent &e) {
            event = e.event;
            beat = e.beat;
        }
        BeatEvent &operator=(const BeatEvent &e) {
            event = e.event;
            beat = e.beat;
            return *this;
        }
        void Save(BinStream &);
        void Load(BinStream &);

        /** "The event argument for the {clip_event <event> <clip>}
            message exported to the controlling Character" */
        Symbol event;
        /** "Clip Beat the event should trigger" */
        float beat;
    };

    class FacingSet {
    public:
        struct FacingBones : public CharBones {
            FacingBones() {}
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

    /** "Blend mode, if any, to use by default for this clip" */
    enum DefaultBlend {
        kPlayNoDefault = 0,
        kPlayNow = 1,
        kPlayNoBlend = 2,
        kPlayFirst = 3,
        kPlayLast = 4,
        kPlayDirty = 8
    };

    /** "Looping mode, if any, to use by default for this clip" */
    enum DefaultLoop {
        kPlayNoLoop = 0x10,
        kPlayLoop = 0x20,
        kPlayGraphLoop = 0x30,
        kPlayNodeLoop = 0x40
    };

    /** "Time units/alignment, if any, for this clip" */
    enum BeatAlignMode {
        kPlayBeatTime = 0,
        kPlayRealTime = 0x200,
        kPlayUserTime = 0x400,
        kPlayBeatAlign1 = 0x1000,
        kPlayBeatAlign2 = 0x2000,
        kPlayBeatAlign4 = 0x4000,
        kPlayBeatAlign8 = 0x8000
    };

    virtual ~CharClip();
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
        MemHeapTracker tmp(_x);
        return MemAlloc(s, __FILE__, 0x51, StaticClassName().Str(), 0);
    }
    static void *operator new(unsigned int s, void *place) { return place; }
    static void operator delete(void *v) {
        MemFree(v, __FILE__, 0x51, StaticClassName().Str());
    }

    /** "Start beat, beat this clip starts at" */
    float StartBeat() const { return mBeatTrack.front().value; }
    /** "End beat, beat this clip ends at" */
    float EndBeat() const { return mBeatTrack.back().value; }
    /** "Length in beats" */
    float LengthBeats() const { return EndBeat() - StartBeat(); }
    /** "Number of original samples taken, pre-keyframe compression" */
    int NumFrames() const {
        return Max<int>(Max<int>(1, mFull.NumSamples()), mFull.NumFrames());
    }
    int InGroups();
    bool SharesGroups(CharClip *);
    float LengthSeconds() const;
    float AverageBeatsPerSecond() const;
    void SetFlags(int);
    void SetPlayFlags(int);
    void SetDefaultBlend(int);
    void SetDefaultLoop(int);
    void SetBeatAlignMode(int);
    void SetRelative(CharClip *);
    void SortEvents();
    int AllocSize();
    void *GetChannel(Symbol);
    void ScaleDown(CharBones &bones, float f);
    int GetContext() const;
    const CharGraphNode *FindFirstNode(CharClip *clip, float beat) const;
    const CharGraphNode *FindLastNode(CharClip *clip, float beat) const;
    const CharGraphNode *FindNode(CharClip *clip, float f1, int iii, float f2) const;
    void EvaluateChannel(void *v1, const void *v2, int iii, float f);
    void ScaleAddSample(CharBones &bones, float f1, int i1, float f2, int i2, float f3);
    float FrameToBeat(float frame) const;
    float BeatToFrame(float beat) const;
    float DeltaSecondsToDeltaBeat(float f1, float beat);
    int BeatToSample(float f, float *fp) const;
    void EvaluateChannel(void *v1, const void *v2, float f3);
    void RotateBy(CharBones &, float);
    void RotateTo(CharBones &, float, float);
    void ScaleAdd(CharBones &, float, float, float);
    void ApplyBlendedSkeletons(CharClip **, CharBones &, float, float);
    void ListBones(std::list<CharBones::Bone> &bones);
    float SampleToBeat(int) const;
    void StuffBones(CharBones &);
    void PoseMeshes(ObjectDir *, float);

    static const float kBeatAccuracy;
    static DataNode GetClipEvents();
    static void LockAndDelete(const CharClip **, int, int);

protected:
    CharClip();

    void Relativize();
    int TransitionVersion();
    DataNode OnGroups(DataArray *);
    DataNode OnHasGroup(DataArray *);

    static void SetDefaultBlendFlag(int &mask, int blendFlag) {
        mask = mask & 0xfffffff0 | blendFlag;
    }
    static void SetDefaultLoopFlag(int &mask, int loopFlag) {
        mask = mask & 0xffffff0f | loopFlag;
    }
    static void SetDefaultBeatAlignModeFlag(int &mask, int alignFlag) {
        mask = mask & 0xffff09ff | alignFlag;
    }

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
