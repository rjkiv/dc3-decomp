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
#include "utl/Symbol.h"

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

    NEW_OBJ(CharDriver);

    ObjectDir *ClipDir() const { return mClips; }
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
    CharClip *FindClip(DataNode const &, bool);

protected:
    CharDriver();

    DataNode OnSetFirstBeatOffset(DataArray *);
    DataNode OnPrint(DataArray const *);
    DataNode OnPlayGroup(const DataArray *);
    DataNode OnPlayGroupFlags(const DataArray *);
    DataNode OnGetClipOrGroupList(DataArray *);
    void SyncInternalBones();
    DataNode OnPlay(const DataArray *);
    DataNode OnSetDefaultClip(DataArray *);
    float Display(float);

    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x30
    /** "pointer to clips object" */
    ObjPtr<ObjectDir> mClips; // 0x44
    CharClipDriver *mFirst; // 0x58
    ObjPtr<CharClip> unk5c;
    ObjPtr<Hmx::Object> mDefaultClip; // 0x70
    ObjPtr<CharClipGroup> unk84;
    bool unk98;
    Symbol unk9c;
    DataNode mLastNode; // 0xa0
    float mOldBeat; // 0xa8
    bool mRealign; // 0xac
    float mBeatScale; // 0xb0
    float mBlendWidth; // 0xb4
    Symbol mClipType; // 0xb8
    ApplyMode mApply; // 0xbc
    CharBonesAlloc *mInternalBones; // 0xc0
    bool mPlayMultipleClips; // 0xc4
    std::map<CharClip *, float> unkc8;
};
