#pragma once
#include "HamRegulate.h"
#include "char/CharClip.h"
#include "char/CharEyes.h"
#include "char/CharLipSync.h"
#include "char/CharServoBone.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "char/Waypoint.h"
#include "hamobj/HamDriver.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

enum HamBackupDancers {
    kBackupDancersRegular = 0,
    kBackupDancersDanceBattle = 1,
    kBackupDancersFinale = 2,
    kBackupDancersOverride = 3,
    kBackupDancersNumTypes = 4
};

/** "Hammer main character class, can be configured to look like characters in /dancer" */
class HamCharacter : public Character {
public:
    enum ClipFlags {
        kNoRetarget = 1,
        kAnimatePropBone = 2,
        kNoRestStep = 4,
        kForceRestStep = 8
    };
    enum Gender {
        /** "female character" */
        kHamFemale = 0,
        /** "male character" */
        kHamMale = 1
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
    REGISTER_OBJ(HamCharacter)
    static void Init();
    static void Terminate();

    bool IsLoading();
    Symbol Outfit() const { return mOutfit; }
    void SetOutfit(Symbol outfit);
    void SetOutfitDir(Symbol dir);
    void UnloadAll();
    Waypoint *GetWaypoint() const { return mWaypoint; }
    void BlendInFaceOverrides(float ms);
    void BlendOutFaceOverrides(float ms);
    void SetFaceOverrideWeight(float w);
    void SetFaceOverrideClip(Symbol name, bool fail);
    Symbol GetFaceOverrideClip();
    float GetFaceOverrideWeight();
    void BlendInFaceOverrideClip(Symbol name, float weight, float blendMs);
    void ResetFaceOverrideBlending();
    // public: CharLipSyncDriver* GetLipSyncDriver();
    void ResyncLipSync(CharLipSync *lipsync);
    void PlayBaseViseme();
    void EnableFacialAnimation(CharLipSync *lipsync, float songOffset);
    void DisableFacialAnimation();
    void SetBlinking(bool blinking);
    void SetLipsyncOffset(float offset);
    ObjectDir *GetNeutralSkeleton();
    int SongAnimation();
    HamDriver *SongDriver();
    HamRegulate *Regulator();
    bool InClipTest();
    // public: void SetUsedRestStep(CharClip*);
    // public: CharClip* UsedRestStep() const;
    void SetUseCameraSkeleton(bool useSkel);
    bool UseCameraSkeleton() const { return mUseCameraSkeleton; }
    void SetIKEffectorWeights(float weight);
    void SetTexBlendersActive(bool active);
    void SetPollWhenHidden(bool poll) { mPollWhenHidden = poll; }
    void SetCampaignVo(const char *pCampaignVO);
    String GetCampaignVo();
    // public: Hmx::Object* GetCampaignVoBank();
    String GetCampaignVoMilo();

    void StartLoad(bool async);
    void ResetFacialAnimation();

    static bool sLoadVO;
    static const int kNumSkeletons = 13;

protected:
    virtual void Load(BinStream &);
    virtual void AddedObject(Hmx::Object *);
    virtual void RemovingObject(Hmx::Object *);

    DataNode OnConfigureFileMerger(DataArray *);
    DataNode OnCamTeleport(DataArray *);
    DataNode OnPostDelete(DataArray *);
    DataNode OnToggleInterestDebugOverlay(DataArray *);
    DataNode OnSoundPlay(const DataArray *);

    void SetCrewCardShowing(bool showing) {
        if (mCrewCard) {
            mCrewCard->SetShowing(showing);
        }
    }
    bool GetCrewCardShowing() const { return mCrewCard && mCrewCard->Showing(); }
    void SetPropShowing(int prop, bool show);
    bool GetPropShowing(int prop);
    void ApplyBlendedSkeletons(HamDriver *driver, CharClip *clip, float weight);

    static CharClip *sSkeletonClips[kNumSkeletons];

    String mCampaignVO; // 0x2d8
    Hmx::Object *mCampaignVOBankDir; // 0x2e0
    ObjectDir *mCampaignVODir; // 0x2e4
    FileMerger *mFileMerger; // 0x2e8
    /** "which character to look like" */
    Symbol mOutfit; // 0x2ec
    Waypoint *mWaypoint; // 0x2f0
    /** "where to load outfits from" */
    Symbol mOutfitDir; // 0x2f4
    bool mIsBackup; // 0x2f8
    /** "Draws a 6 foot square box around the character teleport point" */
    bool mShowBox; // 0x2f9
    bool mOutfitLoaded; // 0x2fa
    ObjPtr<CharEyes> mEyes; // 0x2fc
    /** "Gender of this character" */
    Gender mGender; // 0x310
    CharClip *mUsedRestStep; // 0x314
    /** "Updates the character's animation even though showing is set to FALSE.
        Useful for rendering the character to a texture." */
    bool mPollWhenHidden; // 0x318
    /** "True if the internal TexBlenders are working." */
    bool mTexBlendersActive; // 0x319
    ObjPtrList<CharWeightable> mEffectors; // 0x31c
    float mStoredSongOffset; // 0x330
    ObjectDir *mNeutralSkelDir; // 0x334
    CharServoBone *mSkeletonBones; // 0x338
    ObjPtr<RndMesh> mCrewCard; // 0x33c
    bool mUseCameraSkeleton; // 0x350
};
