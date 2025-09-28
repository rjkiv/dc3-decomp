#pragma once
#include "char/CharEyes.h"
#include "char/CharLipSync.h"
#include "char/CharServoBone.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "char/Waypoint.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"

enum HamBackupDancers {
    kBackupDancersNumTypes = 4
};

enum HamGender {
    /** "female character" */
    kHamFemale = 0,
    /** "male character" */
    kHamMale = 1
};

/** "Hammer main character class, can be configured to look like characters in /dancer" */
class HamCharacter : public Character {
public:
    enum {
        kNumSkeletons = 13
    };

    HamCharacter();
    // Hmx::Object
    virtual ~HamCharacter();
    OBJ_CLASSNAME(Character);
    OBJ_SET_TYPE(Character);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects();
    // RndDrawable
    virtual void Draw();
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    // Character
    virtual CharEyes *GetEyes() { return mEyes; }

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(HamCharacter)
    static void Init();
    static void Terminate();

    void StartLoad(bool);
    void SetTexBlendersActive(bool);
    void SetLipsyncOffset(float);
    void EnableFacialAnimation(CharLipSync *, float);
    void SetBlinking(bool);
    void SetCampaignVo(const char *);
    void SetPropShowing(int prop, bool show) {
        if (mShowableProps.size() > prop) {
            if (mShowableProps[prop])
                mShowableProps[prop]->SetShowing(show);
        }
    }
    String GetCampaignVo();
    void SetOutfit(Symbol);
    void SetOutfitDir(Symbol);
    void UnloadAll();
    String GetCampaignVoMilo();
    bool IsLoading();
    bool InClipTest();
    void SetIKEffectorWeights(float);
    void ResyncLipSync(CharLipSync *);
    void PlayBaseViseme();
    void DisableFacialAnimation();
    void ResetFacialAnimation();
    void BlendInFaceOverrides(float);
    void BlendOutFaceOverrides(float);
    void SetFaceOverrideWeight(float);
    float GetFaceOverrideWeight();
    void SetUseCameraSkeleton(bool);
    Symbol GetFaceOverrideClip();
    void ResetFaceOverrideBlending();
    int SongAnimation();
    ObjectDir *GetNeutralSkeleton();
    void SetFaceOverrideClip(Symbol, bool);

protected:
    virtual void AddedObject(Hmx::Object *);
    virtual void RemovingObject(Hmx::Object *);

    bool GetPropShowing(int);

    DataNode OnConfigureFileMerger(DataArray *);
    DataNode OnCamTeleport(DataArray *);
    DataNode OnPostDelete(DataArray *);
    DataNode OnSoundPlay(const DataArray *);
    DataNode OnToggleInterestDebugOverlay(DataArray *);

    static CharClip *sSkeletonClips[kNumSkeletons];

    String mCampaignVO; // 0x2d8
    Hmx::Object *mCampaignVOBank; // 0x2e0
    ObjectDir *mCampaignVODir; // 0x2e4
    FileMerger *mFileMerger; // 0x2e8
    /** "which character to look like" */
    Symbol mOutfit; // 0x2ec
    Waypoint *mWaypoint; // 0x2f0
    /** "where to load outfits from" */
    Symbol mOutfitDir; // 0x2f4
    bool unk2f8; // 0x2f8
    /** "Draws a 6 foot square box around the character teleport point" */
    bool mShowBox; // 0x2f9
    bool unk2fa; // 0x2fa
    ObjPtr<CharEyes> mEyes; // 0x2fc
    /** "Gender of this character" */
    HamGender mGender; // 0x310
    int unk314; // 0x314
    /** "Updates the character's animation even though showing is set to FALSE.
        Useful for rendering the character to a texture." */
    bool mPollWhenHidden; // 0x318
    /** "True if the internal TexBlenders are working." */
    bool mTexBlendersActive; // 0x319
    ObjPtrList<CharWeightable> mIKEffectors; // 0x31c
    float unk330; // 0x330 - song offset?
    ObjectDir *mNeutralSkelDir; // 0x334
    CharServoBone *mSkeletonBones; // 0x338
    ObjPtr<RndMesh> mCrewCardMesh; // 0x33c
    bool mUseCameraSkeleton; // 0x350
};
