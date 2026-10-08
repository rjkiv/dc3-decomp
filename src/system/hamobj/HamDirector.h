#pragma once
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "char/CharClip.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamVisDir.h"
#include "hamobj/MoveDir.h"
#include "hamobj/MoveGraph.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/PostProc.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "rndobj/Trans.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"
#include "utl/Song.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"
#include "world/CameraManager.h"
#include "world/Dir.h"

class AnimPtr;

class OfflineCallback : public SongCallback {
public:
    OfflineCallback() {}
    virtual ~OfflineCallback() {}
    virtual void SongSetFrame(class Song *, float) {}
    virtual ObjectDir *SongMainDir();
    virtual void SongPlay(bool) {}
    virtual void UpdateObject(const Hmx::Object *, DataArray *) {}
    virtual void Preload() {}
    virtual void ProcessBookmarks(DataNode) {}
};

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
    bool IsMoveMergerFinished() const;
    void HandleDifficultyChange();
    void CheckBeginFatal(int, HamMove *, int);
    void LoadCrew(Symbol, Symbol);
    void SetPhraseMetersFlipped(bool);
    void PoseIconMan(CharClip *, float, RndTex *, bool, CharClip *, float, float);
    void PoseIconMan(const BaseSkeleton *, RndTex *);
    void CleanOriginalMoveData();
    float BeatFromTag(Symbol);
    void UnloadMergers();
    void RemapSongAnimToTempoMap(TempoMap *);

    void DrawIconMan(Symbol, Symbol, Symbol, float, float, RndTex *);
    void DrawIconMan(Difficulty, float, float, float, float, RndTex *);

    ObjectDir *ClipDir() const { return mSongClips; }
    bool NoTransitions() const { return mNoTransitions; }
    MoveDir *GetMoveDir() const { return static_cast<MoveDir *>(mMovesDir.Ptr()); }
    FileMerger *GetMerger() const { return mMerger; }
    ObjectDir *MergerDir() const { return mMerger ? mMerger->Dir() : nullptr; }
    HamCamShot *CurShot() const { return mCurShot; }
    FileMerger *GetGameModeMerger() const { return mGameModeMerger; }
    void SetPickingDisabled(bool disable) { mDisablePicking = disable; }
    bool Unk33d() const { return unk33d; }
    void SetUnk2AC(bool b) { unk2ac = b; }
    PoseFatalities *GetPoseFatalities() const { return mPoseFatalities; }
    RndPostProc *GetUnk18c() const { return unk18c; }
    int StartLoopMargin() const { return mStartLoopMargin; }
    int EndLoopMargin() const { return mEndLoopMargin; }
    Symbol PracticeStart() const { return mPracticeStart; }
    Symbol PracticeEnd() const { return mPracticeEnd; }

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
        mPausePlayerFreestyle = pause;
        if (mVisualizer)
            mVisualizer->SetShowing(!pause);
    }
    void Initialize();
    void HideBackups(bool, bool);
    void RestoreBackups();
    void TeleportChars();
    void OnPopulateMoves();
    void OnPopulateMoveMgr();
    void OnPopulateFromMoveMgr();
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
    bool ReactToCollision(float);
    bool ShouldDoCollisionPrevention() const;
    void StartStopVisualizer();
    void SendCurWorldMsg(Symbol, bool);
    bool ShotsDisabled();
    bool SongAnimation();
    void SyncScene();
    void SetNewWorld();
    void
    UpdatePostProcOverlay(const char *, const RndPostProc *, const RndPostProc *, float);
    ObjectDir *GetDifficultyProxy(Difficulty);
    CharClip *
    GetClipStartAndEndBeats(Symbol, float &, float &, std::pair<float, float> *);
    void ChangeNextShotIfCharacterCollisionLikely();
    bool AreCharactersColliding();

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

    ObjDirPtr<RndDir> mChars; // 0x48
    std::map<Difficulty, AnimPtr> mSongAnims; // 0x5c
    std::map<Difficulty, AnimPtr> mDancerFaceAnims; // 0x74
    ObjPtr<RndPropAnim> mMasterClipAnim; // 0x8c
    ObjPtr<RndPropAnim> mRoutineBuilderAnim0; // 0xa0
    ObjPtr<RndPropAnim> mRoutineBuilderAnim1; // 0xb4
    float mFaceWeight; // 0xc8
    Symbol mFaceViseme; // 0xcc
    /** "How much backup dancers drift, 0 is none, 1 is full" */
    float mBackupDrift; // 0xd0
    ObjPtr<FileMerger> mMerger; // 0xd4
    ObjPtr<FileMerger> mMoveMerger; // 0xe8
    ObjPtr<FileMerger> mGameModeMerger; // 0xfc
    ObjPtr<WorldDir> mVenue; // 0x110
    ObjPtr<SongCollision> mSongCollision; // 0x124
    Symbol mForceVenue; // 0x138
    Symbol mForceScene; // 0x13c
    bool mPickNewShot; // 0x140
    /** "how many have failed" */
    int mNumPlayersFailed; // 0x144
    /** "excitement level" */
    int mExcitement; // 0x148
    bool mSyncScene; // 0x14c
    ObjPtr<RndPostProc> mWorldPostProc; // 0x150
    /** "camera postproc override.  If set, does no postproc blends" */
    ObjPtr<RndPostProc> mCamPostProc; // 0x164
    ObjPtr<RndPostProc> mForcePostProc; // 0x178
    ObjPtr<RndPostProc> unk18c; // 0x18c
    float mForcePostProcBlend; // 0x1a0
    float mForcePostProcBlendRate; // 0x1a4
    ObjPtr<RndPostProc> mPostProcA; // 0x1a8
    ObjPtr<RndPostProc> mPostProcB; // 0x1bc
    float mPostProcBlend; // 0x1d0
    float mVizNumSec; // 0x1d4
    ObjPtr<RndPostProc> mPrevForcePostProc; // 0x1d8
    ObjPtr<RndPostProc> mVizStartPostProc; // 0x1ec
    /** "TRUE if freestyle is allowed" */
    bool mFreestyleEnabled; // 0x200
    ObjPtr<HamCharacter> mPlayer0; // 0x204
    ObjPtr<HamCharacter> mPlayer1; // 0x218
    ObjPtr<HamCharacter> mBackup0; // 0x22c
    ObjPtr<HamCharacter> mBackup1; // 0x240
    bool mBackupsHidden; // 0x254
    /** 0-1 = players 0-1, 2-3 = backups 0-1 */
    bool mBackupState[4]; // 0x255
    bool mDisabled; // 0x259
    /** Whether or not to load asynchronously. */
    bool mAsync; // 0x25a
    /** "currently shown camshot, nice for debugging." */
    ObjPtr<HamCamShot> mCurShot; // 0x25c
    ObjPtr<HamCamShot> mNextShot; // 0x270
    ObjPtr<HamCamShot> mIntroShot; // 0x284
    /** "HamCamShot category" */
    Symbol mShotCategory; // 0x298
    float mNextShotSec; // 0x29c
    bool mDisablePicking; // 0x2a0
    bool unk2a1; // 0x2a1
    int unk2a4; // 0x2a4
    float unk2a8; // 0x2a8
    bool unk2ac; // 0x2ac
    Keys<DircutEntry, DircutEntry> mDirCutKeys; // 0x2b0
    bool mPlayerFreestyle; // 0x2bc
    bool mPausePlayerFreestyle; // 0x2bd
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
    float mLastFrame; // 0x2e4
    /** "If > 0, is which clip to show by itself rather than doing full blending" */
    int mBlendDebug; // 0x2e8
    int mBlendCount; // 0x2ec
    Symbol mClip; // 0x2f0
    /** Each player's current character/outfit (i.e. aubrey02). */
    Symbol mCharacter[2]; // 0x2f4
    /** Each player's current crew. */
    Symbol mCrew[2]; // 0x2fc
    HamBackupDancers mBackupDancers; // 0x304
    ObjPtr<ObjectDir> mSongClips; // 0x308
    ObjPtr<ObjectDir> mMovesDir; // 0x31c
    /** How fast the crowd should move. Options are "slow", "medium", "fast". */
    Symbol mTempo; // 0x330
    /** "If true, does not play transitions" */
    bool mNoTransitions; // 0x334
    /** "If true, check character collisions when picking cam shots" */
    bool mCollisionChecks; // 0x335
    bool mLoadedNewSong; // 0x336
    PoseFatalities *mPoseFatalities; // 0x338
    bool unk33c; // 0x33c - camshot flag
    bool unk33d; // 0x33d
    ObjPtr<Character> mIconManChar; // 0x340
    ObjPtr<RndTexRenderer> mIconManTex; // 0x354
    bool unk368; // 0x368
    bool unk369; // 0x369
    Song *mOfflineSong; // 0x36c
    std::set<Hmx::Object *> unk370; // 0x370
};

extern HamDirector *TheHamDirector;

class AnimPtr : public ObjPtr<RndPropAnim> {
public:
    AnimPtr() : ObjPtr<RndPropAnim>(TheHamDirector) {}
    AnimPtr(RndPropAnim *anim) : ObjPtr<RndPropAnim>(TheHamDirector, anim) {}
};

bool AreDancersColliding1D(
    std::vector<RndTransformable *> &,
    std::vector<RndTransformable *> &,
    const Vector3 &,
    const Vector3 &
);
