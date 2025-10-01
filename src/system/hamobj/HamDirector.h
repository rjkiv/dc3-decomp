#pragma once
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamVisDir.h"
#include "hamobj/MoveGraph.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/PostProc.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/TexRenderer.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "world/CameraManager.h"
#include "world/Dir.h"

class AnimPtr;

/** "Hammer Director, sits in each song file and manages camera + scene changes" */
class HamDirector : public RndPollable, public RndDrawable {
public:
    struct DircutEntry {
        HamCamShot *unk0;
        bool unk4;
    };
    // Hmx::Object
    virtual ~HamDirector();
    OBJ_CLASSNAME(HamDirector);
    OBJ_SET_TYPE(HamDirector);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void ListPollChildren(std::list<RndPollable *> &) const;
    // RndDrawable
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual void CollideList(const Segment &, std::list<Collision> &);

    OBJ_MEM_OVERLOAD(0x6D)
    NEW_OBJ(HamDirector)

    WorldDir *GetWorld();

    float GetMainFaceOverrideWeight();
    void SetMainFaceOverrideWeight(float);
    Symbol GetMainFaceOverrideClip() const;
    void SetMainFaceOverrideClip(Symbol);
    bool IsWorldLoaded() const;
    void UnloadAll();
    void ForceScene(Symbol);
    void ForceMiniVenue(Symbol);
    void ReselectWorldPostProc();
    void PlayCharBaseVisemes();
    void EnableFacialAnimation();
    void DisableFacialAnimation();
    void ResetFacialAnimation();
    void SetLipsyncOffsets(float);
    void ResyncFaceDrivers();
    void BlendInFaceOverrides(float);
    void BlendOutFaceOverrides(float);
    Symbol MoveNameFromBeat(float, int);
    RndPropAnim *SongAnim(int);
    RndPropAnim *SongAnimByDifficulty(Difficulty);
    RndPropAnim *DancerFaceAnimByPlayer(int);
    void Reteleport();
    void StartStopVisualizer(bool, int);
    void SetPlayerSpotlightsEnabled(bool);
    void ChangePlayerCharacter(int, Symbol, Symbol, Symbol);
    void InitOffline();
    void OfflineLoadSong(Symbol);
    void DrawDebug();
    void ArmMultiIntroMode();
    void PlayIntroShot();
    void SetupAnims();
    RndPropAnim *GetPropAnim(Difficulty, const char *, bool);
    void SetupRoutineBuilderAnims();
    PropKeys *GetPropKeys(Difficulty, Symbol);
    void VenueEnter(WorldDir *);
    void ForceShot(const char *);
    PropKeys *GetMasterKeys(Symbol);
    Key<Symbol> *GetMasterPracticeFrame(Symbol);
    PropKeys *GetPropKeysByPlayer(int, Symbol);
    void TriggerNextIntro();
    WorldDir *GetVenueWorld();
    void UnselectVisualizerPostProc();
    bool GetPracticeFrames(Key<Symbol> *&, Key<Symbol> *&);
    HamCharacter *GetCharacter(int) const;
    HamCharacter *GetBackup(int);
    void LoadRoutineBuilderData(std::set<const MoveVariant *> &, bool);
    bool InPracticeMode();
    void MoveKeys(Difficulty, class MoveDir *, std::vector<HamMoveKey> &);

    DataNode OnGetDancerVisemes(DataArray *);

protected:
    HamDirector();

    void SetShot(Symbol);

    /** World event options: (none bonusfx bonusfx_optional chorus verse) */
    void SetWorldEvent(Symbol);

    Symbol ClosestMove();
    void UpdatePlayerFreestyle(bool);
    void SetCharSpot(Symbol, Symbol);
    void PausePlayerFreestyle(bool pause) {
        mPlayerFreestylePaused = pause;
        if (mVisualizer)
            mVisualizer->SetShowing(!pause);
    }
    void Initialize();
    void HideBackups(bool, bool);
    void RestoreBackups();
    void TeleportChars();
    void OnPopulateMoves();
    void OnPopulateMoveMgr();
    void OnPopulateFromFile();
    void HudEntered();
    void PickIntroShot();
    void FindNextShot();
    void PlayNextShot();
    void SetMasterClipAnim();
    HamCamShot *FindNextDircut();
    void SetDircut(Symbol, std::vector<CameraManager::PropertyFilter>);
    void AddNumPlayers(std::vector<CameraManager::PropertyFilter> &, DataArray *);
    void ReactToCollision_InsertRealShot(Symbol, float);
    void ReactToCollision_MoveShot(int, float);
    bool ShouldDoCollisionPrevention() const;
    void StartStopVisualizer();
    void SendCurWorldMsg(Symbol, bool);
    bool ShotsDisabled();
    bool SongAnimation();
    void SyncScene();
    void SetNewWorld();
    ObjectDir *GetDifficultyProxy(Difficulty);

    DataNode OnShotOver(DataArray *);
    DataNode OnPostProcInterp(DataArray *);
    DataNode OnSaveSong(DataArray *);
    DataNode OnSaveFaceAnims(DataArray *);
    DataNode OnFileLoaded(DataArray *);
    DataNode OnFileMerged(DataArray *);
    DataNode OnLoadSong(DataArray *);
    DataNode OnSelectCamera(DataArray *);
    DataNode OnCycleShot(DataArray *);
    DataNode OnForceShot(DataArray *);
    DataNode OnPostProcs(DataArray *);
    DataNode OnSetDircut(DataArray *);
    DataNode OnBlendInFaceClip(DataArray *);
    DataNode OnPracticeBeats(DataArray *);
    DataNode OnToggleCamshotFlag();
    DataNode OnListPossibleMoves();
    DataNode OnListPossibleVariants();
    DataNode OnClipAnnotate(DataArray *);
    DataNode OnClipSafeToAdd(DataArray *);
    DataNode OnClipList(DataArray *);
    DataNode OnPracticeSafeToAdd(DataArray *);
    DataNode OnPracticeAnnotate(DataArray *);
    DataNode PracticeList(Difficulty);
    DataNode OnToggleDebugInterests(DataArray *);
    DataNode OnToggleCamCharacterSkeleton(DataArray *);

    ObjDirPtr<RndDir> unk48; // 0x48
    std::map<Difficulty, AnimPtr> mSongAnims; // 0x5c
    std::map<Difficulty, AnimPtr> mDancerFaceAnims; // 0x74
    ObjPtr<RndPropAnim> mMasterClipAnim; // 0x8c
    ObjPtr<RndPropAnim> mPlayer1RoutineBuilderAnim; // 0xa0
    ObjPtr<RndPropAnim> mPlayer2RoutineBuilderAnim; // 0xb4
    float unkc8; // 0xc8
    Symbol unkcc; // 0xcc
    /** "How much backup dancers drift, 0 is none, 1 is full" */
    float mBackupDrift; // 0xd0
    ObjPtr<FileMerger> mMerger; // 0xd4
    ObjPtr<FileMerger> mMoveMerger; // 0xe8
    ObjPtr<FileMerger> mGameModeMerger; // 0xfc
    ObjPtr<WorldDir> mVenue; // 0x110
    ObjPtr<SongCollision> unk124; // 0x124
    Symbol unk138; // 0x138
    Symbol unk13c; // 0x13c
    bool unk140; // 0x140
    /** "how many have failed" */
    int mNumPlayersFailed; // 0x144
    /** "excitement level" */
    int mExcitement; // 0x148
    bool unk14c; // 0x14c
    ObjPtr<RndPostProc> mWorldPostProc; // 0x150
    /** "camera postproc override.  If set, does no postproc blends" */
    ObjPtr<RndPostProc> mCamPostProc; // 0x164
    ObjPtr<RndPostProc> mForcePostProc; // 0x178
    ObjPtr<RndPostProc> unk18c; // 0x18c
    float mForcePostProcBlend; // 0x1a0
    float mForcePostProcBlendRate; // 0x1a4
    ObjPtr<RndPostProc> unk1a8; // 0x1a8
    ObjPtr<RndPostProc> unk1bc; // 0x1bc
    float unk1d0; // 0x1d0
    float unk1d4; // 0x1d4
    ObjPtr<RndPostProc> unk1d8; // 0x1d8
    ObjPtr<RndPostProc> mVisualizerPostProc; // 0x1ec
    /** "TRUE if freestyle is allowed" */
    bool mFreestyleEnabled; // 0x200
    ObjPtr<HamCharacter> mPlayer0Char; // 0x204
    ObjPtr<HamCharacter> mPlayer1Char; // 0x218
    ObjPtr<HamCharacter> mBackup0Char; // 0x22c
    ObjPtr<HamCharacter> mBackup1Char; // 0x240
    bool unk254; // 0x254
    bool unk255[4]; // 0x255
    bool mDisabled; // 0x259
    bool unk25a;
    /** "currently shown camshot, nice for debugging." */
    ObjPtr<HamCamShot> mCurShot; // 0x25c
    ObjPtr<HamCamShot> mNextShot; // 0x270
    ObjPtr<HamCamShot> unk284; // 0x284
    /** "HamCamShot category" */
    Symbol mShot; // 0x298
    float unk29c; // 0x29c
    bool mDisablePicking; // 0x2a0
    bool unk2a1; // 0x2a1
    int unk2a4; // 0x2a4
    float unk2a8; // 0x2a8
    bool unk2ac; // 0x2ac
    Keys<DircutEntry, DircutEntry> mDirCutKeys; // 0x2b0
    bool mPlayerFreestyle; // 0x2bc
    bool mPlayerFreestylePaused; // 0x2bd
    ObjPtr<HamVisDir> mVisualizer; // 0x2c0
    /** "start frame of practice mode" */
    Symbol mPracticeStart; // 0x2d4
    /** "end frame of practice mode" */
    Symbol mPracticeEnd; // 0x2d8
    /** "In practice mode, measures before practice_start until loop".
        Ranges from 1 to 100. */
    int mStartLoopMargin; // 0x2dc
    /** "In practice mode, measures after practice_end until loop".
        Ranges from 1 to 100. */
    int mEndLoopMargin; // 0x2e0
    int unk2e4; // 0x2e4
    /** "If > 0, is which clip to show by itself rather than doing full blending" */
    int mBlendDebug; // 0x2e8
    int unk2ec; // 0x2ec
    Symbol unk2f0; // 0x2f0
    Symbol unk2f4[2]; // 0x2f4
    Symbol unk2fc[2]; // 0x2fc
    HamBackupDancers mBackupDancers; // 0x304
    ObjPtr<ObjectDir> mClipDir; // 0x308
    ObjPtr<ObjectDir> mMoveDir; // 0x31c
    Symbol unk330; // 0x330
    /** "If true, does not play transitions" */
    bool mNoTransitions; // 0x334
    /** "If true, check character collisions when picking cam shots" */
    bool mCollisionChecks; // 0x335
    bool mLoadedNewSong; // 0x336
    PoseFatalities *mPoseFatalities; // 0x338
    bool unk33c; // 0x33c
    bool unk33d; // 0x33d
    ObjPtr<Character> mIconManChar; // 0x340
    ObjPtr<RndTexRenderer> mIconManTex; // 0x354
    bool unk368; // 0x368
    bool unk369; // 0x369
    int mOfflineSong; // 0x36c - Song*
    std::set<Hmx::Object *> unk370; // 0x370
};

extern HamDirector *TheHamDirector;

class AnimPtr : public ObjPtr<RndPropAnim> {
public:
    AnimPtr() : ObjPtr<RndPropAnim>(TheHamDirector) {}
    AnimPtr(RndPropAnim *anim) : ObjPtr<RndPropAnim>(TheHamDirector, anim) {}
};
