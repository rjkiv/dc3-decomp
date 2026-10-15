#pragma once
#include "DetectFrame.h"
#include "FilterQueue.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/Difficulty.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/MoveDetector.h"
#include "hamobj/PracticeSection.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Data.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Overlay.h"
#include "ui/UILabelDir.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include <set>

/** "Dir for HamMoves, contains debugging functionality" */
class MoveDir : public SkeletonDir, public RndOverlay::Callback {
public:
    // size 0x3c
    class MovePlayerData {
    public:
        MovePlayerData() : mCurMove(nullptr) {}
        void Reset() {
            mCurMove = nullptr;
            mPhraseMeter = nullptr;
            mFeedback = nullptr;
            mTextFeedback = nullptr;
            mState = 0;
        }
        ObjPtr<HamMove> mCurMove; // 0x0
        std::vector<DetectFrame> mDetectFrames; // 0x14
        std::vector<HamMoveKey> mMoveKeys; // 0x20
        int mState; // 0x2c
        HamPhraseMeter *mPhraseMeter; // 0x30
        CharFeedback *mFeedback; // 0x34
        RndDrawable *mTextFeedback; // 0x38
    };
    // Hmx::Object
    virtual ~MoveDir();
    OBJ_CLASSNAME(MoveDir)
    OBJ_SET_TYPE(MoveDir)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    // SkeletonCallback
    virtual void Update(const struct SkeletonUpdateData &);
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &);
    // RndOverlay::Callback
    virtual float UpdateOverlay(RndOverlay *, float f2);

    OBJ_MEM_OVERLOAD(0x2F)
    NEW_OBJ(MoveDir)

    void ClearLimbFeedback(int);
    void SetFiltersEnabled(bool);
    void SetMoveOverlay(bool);
    void SetSongPlayClip(SkeletonClip *);
    HamMove *CurrentMove(int) const;
    int MoveIdx() const;
    int MoveBeat() const;
    void StopSongRecord();
    void FlushMoveRecord();
    void SwapMoveRecord();
    HamMove *GetMoveAtMeasure(int, int);
    DancerSequence *PerformanceSequence(Difficulty);
    void FinishGameRecord();
    void SetupSongRecordClip();
    void SetDancerSequence(DancerSequence *);
    void ResetDetection();
    void ResetDetectFrames(int, Difficulty);
    void SetCurrentMove(int, HamMove *);
    float
    DetectFrac(int, const HamMove *, const std::pair<DetectFrame *, DetectFrame *> &);
    void
    EnqueueDetectFrames(float, int, std::vector<DetectFrame> &, const FilterVersion *);
    void SimulateSong(int, int) {}
    void OnBeat();
    void FinalPoseStateMachine();
    void SetDebugLoop(bool);
    PracticeSection *GetPracticeSection(Difficulty);
    DancerSequence *SkillsSequence(Difficulty, Symbol, Symbol);
    float DetectFrac(int, int);
    bool InGracePeriod(int);

    MoveAsyncDetector *GetAsyncDetector() const { return mAsyncDetector; }

    static void Init();
    static void LoadScoring(const DataArray *);
    static const FilterVersion *FindFilterVersion(FilterVersionType);
    static float SongSeconds();
    static bool sGameRecord;
    static bool sGameRecord2Player;

private:
    void SetFilterVersion(Symbol);
    SkeletonClip *ImportClip(bool);
    void ReloadScoring();
    void DetectRange(
        std::vector<DetectFrame> &, std::pair<DetectFrame *, DetectFrame *> &, int, int
    );
    void PostUpdateFilters();
    float SongSpeed() const;
    MoveFrame *ClosestMoveFrame();

    DataNode OnStreamJump(const DataArray *);
    float DetectRangePSNR(
        const std::pair<const DetectFrame *, const DetectFrame *> &, const FilterVersion *
    ) const;
    float DetectRangeFrac(
        const std::pair<DetectFrame *, DetectFrame *> &, const FilterVersion *
    ) const;

    static std::vector<FilterVersion *> sFilterVersions;
    static float sLatencySeconds;
    static float sPLFMinTimeError;

protected:
    MoveDir();

    virtual void MiloUpdate();

    /** Ham1 (DC1) or Ham2 (DC2/DC3) filters */
    FilterVersion *mFilterVer; // 0x288
    /** "Show debugging overlay for the current HamMove" */
    bool mShowMoveOverlay; // 0x28c
    /** "Types of error nodes to show" */
    /** "Specific joints to display debug viz for" */
    /** A bitmask of ErrorNodeType enums */
    int mErrorNodeInfo; // 0x290
    /** "Clip to play back in sync with the song" */
    ObjPtr<SkeletonClip> mSongPlayClip; // 0x294
    /** "Clip to use for song recording" */
    ObjPtr<SkeletonClip> mSongRecordClip; // 0x2a8
    ObjPtr<SkeletonClip> mSongRecordClip2; // 0x2bc
    ObjPtr<SkeletonClip> mSongRecordClip3; // 0x2d0
    int mMoveRecordClipIdx; // 0x2e4
    /** "If set, report will be limited to this move" */
    ObjPtr<HamMove> mReportMove; // 0x2e8
    /** "The pre-recorded .clp file to import" */
    String mImportClipPath; // 0x2fc
    bool mFiltersEnabled; // 0x304
    Hmx::Object *mGamePanel; // 0x308
    float mLastResultSeconds; // 0x30c
    float mLastMoveSeconds; // 0x310
    FilterQueue *mFilterQueue; // 0x314
    MovePlayerData mMovePlayerData[2]; // 0x318
    MoveAsyncDetector *mAsyncDetector; // 0x390
    DirLoader *mUpdateLoader; // 0x394
    std::list<ObjDirPtr<UILabelDir> > mAltFontDirs; // 0x398
    /** Smoothed normalized results of the current move. */
    DoubleExponentialSmoother mSmoothers[2]; // 0x3a0

    // current move stuffs vs last move stuffs? yes.
    HamMove *mPrevMove[2]; // 0x3c8
    HamMove *mCurMove[2]; // 0x3d0
    float mCurMoveNormalizedResult[2]; // 0x3d8
    float mCurMovePrevNormalizedResult[2]; // 0x3e8
    MoveRating mCurMoveRating[2]; // 0x3e0
    int mCurMovePrevRating[2]; // 0x3f0

    int mFinishingMoveMeasure; // 0x3f8
    RndOverlay *mMoveOverlay; // 0x3fc
    /** Before DC3 MoveGraph-based songs, you had one DancerSequence per song, per
     * difficulty - this probably holds expert. */
    ObjPtr<DancerSequence> mDancerSequence; // 0x400
    const DancerSkeleton *mDrawDancerSkeleton; // 0x414
    SkeletonViz *mDancerViz; // 0x418
    DetectFrame *mVizDetectFrame; // 0x41c
    /** "Offset debug skeleton by latency offset" */
    bool mDebugLatencyOffset; // 0x420
    Skeleton mVizSkeleton; // 0x424
    bool mDebugLoop; // 0xef8
    float mLastPollMs; // 0xefc
    /** "Show collision debug" */
    bool mDebugCollision; // 0xf00
    Transform mDebugCollisionXfms[2]; // 0xf04
    int mDebugCollisionBeat; // 0xf84
    std::set<DetectFrame *> mCurDetectFrames; // 0xf88
};
