#pragma once
#include "DetectFrame.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Object.h"
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
            unk2c = 0;
            unk30 = 0;
            mFeedback = nullptr;
            unk38 = 0;
        }
        ObjPtr<HamMove> mCurMove; // 0x0
        std::vector<DetectFrame> unk14; // 0x14
        std::vector<HamMoveKey> unk20; // 0x20
        int unk2c; // 0x2c
        int unk30; // 0x30
        CharFeedback *mFeedback; // 0x34
        int unk38; // 0x38
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

private:
    void SetFilterVersion(Symbol);

    static std::vector<FilterVersion *> sFilterVersions;

protected:
    MoveDir();

    virtual void MiloUpdate();

    int unk288; // 0x288
    /** "Show debugging overlay for the current HamMove" */
    bool mShowMoveOverlay; // 0x28c
    /** "Types of error nodes to show" */
    /** "Specific joints to display debug viz for" */
    /** A bitmask of ErrorNodeType enums */
    int mErrorNodeInfo; // 0x290
    /** "Clip to play back in sync with the song" */
    ObjPtr<SkeletonClip> mPlayClip; // 0x294
    /** "Clip to use for song recording" */
    ObjPtr<SkeletonClip> mRecordClip; // 0x2a8
    ObjPtr<SkeletonClip> unk2bc; // 0x2bc
    ObjPtr<SkeletonClip> unk2d0; // 0x2d0
    int unk2e4; // 0x2e4
    /** "If set, report will be limited to this move" */
    ObjPtr<HamMove> mReportMove; // 0x2e8
    /** "The pre-recorded .clp file to import" */
    String mImportClipPath; // 0x2fc
    bool mFiltersEnabled; // 0x304
    int unk308; // 0x308
    float unk30c; // 0x30c
    int unk310; // 0x310
    int mFilterQueue; // 0x314 - FilterQueue*
    MovePlayerData mMovePlayerData[2]; // 0x318
    int unk390; // 0x390 - MoveAsyncDetector*
    int unk394; // 0x394
    std::list<ObjDirPtr<UILabelDir> > unk398; // 0x398
    Vector2DESmoother unk3a0; // 0x3a0
    char buffer[0x30];
    int unk3f8; // 0x3f8
    RndOverlay *mMoveOverlay; // 0x3fc
    ObjPtr<DancerSequence> unk400; // 0x400
    int unk414; // 0x414
    SkeletonViz *mSkeletonViz; // 0x418
    int unk41c; // 0x41c
    /** "Offset debug skeleton by latency offset" */
    bool mDebugLatencyOffset; // 0x420
    Skeleton unk424; // 0x424
    bool unkef8; // 0xef8
    float mLastPollMs; // 0xefc
    /** "Show collision debug" */
    bool mDebugCollision; // 0xf00
    Transform unkf04[2]; // 0xf04
    int unkf84; // 0xf84
    std::set<DetectFrame *> unkf88; // 0xf88
};
