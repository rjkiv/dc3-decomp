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
    enum ApplyMode {
        kApplyBlend = 0,
        kApplyAdd = 1,
        kApplyRotateTo = 2,
        kApplyBlendWeights = 3
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
    REGISTER_OBJ(CharDriver);

    void SetClips(ObjectDir *c);
    void SetBones(CharBonesObject *bones);
    ObjectDir *Clips() const { return mClips; }
    CharBonesObject *Bones() const { return mBones; }
    void SetBeatScale(float newScale, bool realign);
    // float BeatScale();
    CharClip *FindClip(const DataNode &, bool notify = true);
    CharClipDriver *
    Play(CharClip *c, int playFlags, float blendWidth, float startFrame, float deltaStart);
    CharClipDriver *Play(
        const DataNode &n,
        int playFlags,
        float blendWidth,
        float startFrame,
        float deltaStart
    );
    CharClipDriver *PlayGroup(
        CharClipGroup *g,
        int playFlags,
        float blendWidth,
        float startFrame,
        float deltaStart
    );
    CharClipDriver *PlayGroup(
        const char *groupName,
        int playFlags,
        float blendWidth,
        float startFrame,
        float deltaStart
    );
    float Beat() { return mOldBeat; }
    float TopClipFrame();
    void SetStarved(Symbol);
    CharClipDriver *First() { return mFirst; }
    CharClipDriver *Last();
    CharClipDriver *Before(CharClipDriver *cur);
    CharClipDriver *FirstPlaying();
    // CharClipDriver *MostPlaying();
    CharClip *FirstClip();
    // CharClip *LastClip();
    CharClip *FirstPlayingClip();
    // CharClip *MostPlayingClip();
    // CharClipDriver *FindClipDriver(CharClip *);
    CharClipGroup *LastGroup() const { return mLastGroup; }
    void Offset(float, float);
    bool Starved();
    void SetBlendWidth(float w) { mBlendWidth = w; }
    float EvaluateFlags(int flags);
    // void ExitUnplaying();
    void Transfer(const CharDriver &cd);
    void Clear();

    void SetClipType(Symbol);
    void SetApply(ApplyMode);
    Symbol GetClipType() const { return mClipType; }
    void SetClipWeightMap();
    const std::map<CharClip *, float> &ClipMap() { return mCurrentClipWeights; }

protected:
    CharDriver();

    DataNode OnSetFirstBeatOffset(DataArray *);
    DataNode OnPrint(const DataArray *);
    DataNode OnPlayGroup(const DataArray *);
    DataNode OnPlayGroupFlags(const DataArray *);
    DataNode OnGetClipOrGroupList(DataArray *);
    DataNode OnPlay(const DataArray *);
    DataNode OnSetDefaultClip(DataArray *);
    DataNode OnGetFirstPlayingFlags(const DataArray *);
    DataNode OnGetFirstFlags(const DataArray *);

    void SyncInternalBones();
    float Display(float top);

    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x30 / -0xb4
    /** "pointer to clips object" */
    ObjPtr<ObjectDir> mClips; // 0x44 / -0xa0
    CharClipDriver *mFirst; // 0x58 / -0x8c
    /** "Pick a clip to play" */
    ObjPtr<CharClip> mTestClip; // 0x5c / -0x88
    /** "Clip or Group played on enter by default" */
    ObjPtr<Hmx::Object> mDefaultClipOrGroup; // 0x70
    ObjPtr<CharClipGroup> mLastGroup; // 0x84
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
    std::map<CharClip *, float> mCurrentClipWeights; // 0xc8
};
