#pragma once
#include "CharLipSync.h"
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "char/CharDriver.h"
#include "char/CharLipSync.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "utl/MemMgr.h"

/** "Drives lip sync animation" */
class CharLipSyncDriver : public RndHighlightable,
                          public CharWeightable,
                          public CharPollable {
public:
    // Hmx::Object
    virtual ~CharLipSyncDriver();
    virtual void Highlight();
    OBJ_CLASSNAME(CharLipSyncDriver);
    OBJ_SET_TYPE(CharLipSyncDriver);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(CharLipSyncDriver)

    void Sync();
    void ClearLipSync();
    void SetClips(ObjectDir *);
    bool SetLipSync(CharLipSync *);
    void ResetOverrideBlend();
    void BlendInOverrideClip(CharClip *, float, float);
    void BlendInOverrides(float);
    void BlendOutOverrides(float);
    void ScaleAddViseme(CharClip *, float);
    void SetSongOffset(float offset) { mSongOffset = offset; }
    void SetOverrideWeight(float weight) { mOverrideWeight = weight; }
    float GetOverrideWeight() const { return mOverrideWeight; }
    CharClip *OverrideClip() const { return mOverrideClip; }
    CharLipSync *LipSync() const { return mLipSync; }

protected:
    CharLipSyncDriver();

    void ApplyBlinks();

    /** "The lipsync file to use" */
    ObjPtr<CharLipSync> mLipSync; // 0x30
    /** "pointer to the visemes" */
    ObjPtr<ObjectDir> mClips; // 0x44
    ObjPtr<CharClip> mBlinkClip; // 0x58
    /** "Will use this song if set, except for blinks" */
    ObjPtr<CharLipSyncDriver> mSongOwner; // 0x6c
    /** "offset within song in seconds, resets on song change" */
    float mSongOffset; // 0x80
    /** "should we loop this song, resets on song change" */
    bool mLoop; // 0x84
    CharLipSync::PlayBack *unk88; // 0x88
    bool unk8c; // 0x8c
    float unk90; // 0x90
    CharLipSync::PlayBack *unk94; // 0x94
    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x98
    /** "Test charclip to apply, does nothing else" */
    ObjPtr<CharClip> mTestClip; // 0xac
    /** "weight to apply this clip with" */
    float mTestWeight; // 0xc0
    float unkc4; // 0xc4
    bool unkc8; // 0xc8
    bool unkc9; // 0xc9
    float unkcc; // 0xcc
    bool unkd0; // 0xd0
    float unkd4; // 0xd4
    /** "default clip to be used as the override - maybe be overriden programatically" */
    ObjPtr<CharClip> mOverrideClip; // 0xd8
    /** "weight to blend override clip. this is mostly here for testing,
        because its likely to be set programatically." */
    float mOverrideWeight; // 0xec
    /** "an optional clipset that provides list of clips to override face with -
        viseme clipset is used otherwise" */
    ObjPtr<ObjectDir> mOverrideOptions; // 0xf0
    /** "is the override clip applied addtively on top of face mocap?
        If false, it will blend." */
    bool mApplyOverrideAdditively; // 0x104
    ObjPtr<CharClip> unk108; // 0x108
    float unk11c; // 0x11c
    float unk120; // 0x120
    float unk124; // 0x124
    bool unk128; // 0x128
    /** "This will be used instead of the song, if set" */
    ObjPtr<CharDriver> mAlternateDriver; // 0x12c
};
