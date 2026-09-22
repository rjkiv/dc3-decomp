#pragma once
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "char/CharClipDriver.h"
#include "char/CharClipGroup.h"
#include "char/CharWeightable.h"
#include "char/CharPollable.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

/** "Class to Drive, Schedule and Blend CharClips.
    Basically a stack, new ones are pushed onto the bottom,
    start playing at some point, and then pop off the ones above it
    once they are fully blended in." */
class CharDriver : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    enum ApplyMode { // from RB3 decomp
        kApplyBlend,
        kApplyAdd,
        kApplyRotateTo,
        kApplyBlendWeights
    };

    // Hmx::Object
    virtual ~CharDriver();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CharDriver);
    OBJ_SET_TYPE(CharDriver);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1C)
    NEW_OBJ(CharDriver);

    ObjectDir *ClipDir() const { return mClips; }
    CharBonesObject *GetBones() const { return mBones; }
    CharClipDriver *First() { return mFirst; }
    CharClipGroup *LastPlayedGroup() const { return mLastPlayedGroup; }
    CharClip *FirstClip();
    CharClipDriver *FirstPlaying();
    CharClipDriver *Last();
    CharClipDriver *Before(CharClipDriver *);
    void Clear();
    void SetBeatScale(float, bool);
    float EvaluateFlags(int);
    CharClip *FirstPlayingClip();
    void SetClips(ObjectDir *);
    float TopClipFrame();
    void Transfer(CharDriver const &);
    void SetBones(CharBonesObject *);
    CharClipDriver *Play(CharClip *, int, float, float, float);
    CharClipDriver *Play(const DataNode &, int, float, float, float);
    CharClipDriver *PlayGroup(CharClipGroup *, int, float, float, float);
    CharClipDriver *PlayGroup(const char *, int, float, float, float);
    void SetClipWeightMap();
    CharClip *FindClip(DataNode const &, bool notify = true);

protected:
    CharDriver();

    DataNode OnSetFirstBeatOffset(DataArray *);
    DataNode OnPrint(DataArray const *);
    DataNode OnPlayGroup(const DataArray *);
    DataNode OnPlayGroupFlags(const DataArray *);
    DataNode OnGetClipOrGroupList(DataArray *);
    DataNode OnPlay(const DataArray *);
    DataNode OnSetDefaultClip(DataArray *);
    DataNode OnGetFirstPlayingFlags(const DataArray *);
    DataNode OnGetFirstFlags(const DataArray *);

    void SyncInternalBones();
    float Display(float);
    void SetClipType(Symbol);
    void SetApply(ApplyMode);
    void SetStarved(Symbol);
    void Offset(float, float);

    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x30 / -0xb4
    /** "pointer to clips object" */
    ObjPtr<ObjectDir> mClips; // 0x44 / -0xa0
    CharClipDriver *mFirst; // 0x58 / -0x8c
    /** "Pick a clip to play" */
    ObjPtr<CharClip> mTestClip; // 0x5c / -0x88
    /** "Clip or Group played on enter by default" */
    ObjPtr<Hmx::Object> mDefaultClip; // 0x70
    ObjPtr<CharClipGroup> mLastPlayedGroup; // 0x84
    /** "If true, plays the default_clip_or_group whenever starved" */
    bool mDefaultPlayStarved; // 0x98 / -0x74
    Symbol mStarvedHandler; // 0x9c
    DataNode mLastNode; // 0xa0
    float mOldBeat; // 0xa8
    /** "Realigns the clips to always be aligned with the beat in the measure" */
    bool mRealign; // 0xac / -0x38
    /** "Scale factor applied to incoming beat" */
    float mBeatScale; // 0xb0
    /** "Width in beats to blend to the next clip" */
    float mBlendWidth; // 0xb4 / -0x30
    /** "What type of clip we can blend" */
    Symbol mClipType; // 0xb8 / -0x2c
    /** "How to apply the driver to its bones" */
    ApplyMode mApply; // 0xbc / -0x28;
    CharBonesAlloc *mInternalBones; // 0xc0
    /** "Turn on to set this driver to play multiple clips" */
    bool mPlayMultipleClips; // 0xc4 / -0x20
    std::map<CharClip *, float> mClipWeightMap; // 0xc8
};
