#pragma once
#include "FreestyleMove.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonViz.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

#define MAX_FREESTYLE_MOVES 4

// size 0x110
class FreestyleMoveRecorder : public SkeletonCallback {
public:
    struct JointAngle {
        JointAngle(SkeletonJoint joint) : mJoint(joint) {}
        SkeletonJoint mJoint;
    };
    struct JointPos {
        JointPos(SkeletonJoint joint, SkeletonCoordSys cs)
            : mJoint(joint), mCoordSys(cs) {}
        SkeletonJoint mJoint; // 0x0
        SkeletonCoordSys mCoordSys; // 0x4
    };
    FreestyleMoveRecorder();
    virtual ~FreestyleMoveRecorder();
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *) {}
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    void Poll();
    void Free();
    void StartRecording();
    void StartRecordingDancerTake();
    void StopRecording();
    void ClearRecording();
    void StartPlayback(bool usePreviousRecording);
    void StopPlayback();
    void ClearDancerTake();
    BaseSkeleton *GetLiveSkeleton();
    float CompareSkeletonPositions(
        const BaseSkeleton *skel1, const BaseSkeleton *skel2, float errorWeight
    ) const;
    void AssignStaticInstance();
    void DrawDebug();
    void PlaybackComplete();
    void ClearFrameScores();
    void ReadFreestyleMoveClip(String name, int &frameCount, FreestyleMoveFrame *frames);
    float GetScore(
        const BaseSkeleton *liveSkel,
        int historySlot,
        float overrideTime,
        bool compareDancerFrames
    );
    float GetScore(int, int, float, bool);

    void SetUnk40(int i) { mPlaybackRep = i; }
    void SetVal44(int i) { mSkeletonIndex = i; } // change once context found
    void SetSongName(Symbol s) { mSongName = s; }
    int GetUnkB8() const { return mCurrentMove; }
    RndTex *GetOutputTex() const { return mOutputTex; }
    int GetDancerTakeFrameCount() const { return mDancerTakeFrameCount; }

    void SetFreestyleMove(int index) {
        MILO_ASSERT(index >= 0 && index < MAX_FREESTYLE_MOVES, 0x50);
        mCurrentMove = index;
    }

    MEM_OVERLOAD(FreestyleMoveRecorder, 0x2E);

private:
    void UpdateRecordingAttempt(const BaseSkeleton *liveSkel, float ms);
    void RecordMoveAttempt(String name);
    void WriteRecordedMoveAttempt();
    void WriteFreestyleMoveClip(String name, int frameCount, FreestyleMoveFrame *frames);
    void ClearFreestyleMoveClip();
    void UpdateFakeSkeleton();
    void CompareDisplacementVectors(
        const Vector3 &v1, int ms1, const Vector3 &v2, int ms2, float &score, float &weight
    ) const;

    static DataNode OnRecordAttempt(DataArray *);
    static DataNode OnWriteCreated(DataArray *);
    static DataNode OnReadCreated(DataArray *);
    static DataNode OnReadAttempt(DataArray *);
    static DataNode OnClearAttempt(DataArray *);

    float mFakeSkeletonTime; // 0x4
    FreestyleMoveFrame *mFakeSkeletonFrames; // 0x8
    int mFakeSkeletonFrameCount; // 0xc
    String mRecordingName; // 0x10
    FreestyleMoveFrame *mRecordedAttemptFrames; // 0x18
    int mRecordedAttemptFrameCount; // 0x1c
    int mDebugAlreadyLoadedMoveFor; // 0x20
    int mMaxFrames; // 0x24
    float mRecordTime; // 0x28
    float mPlayTime; // 0x2c
    float mFrameRate; // 0x30
    int mStopRecordingAt; // 0x34
    bool mRecordingDancerTake; // 0x38
    bool mPlayPreviousRecording; // 0x39
    Symbol mSongName; // 0x3c
    int mPlaybackRep; // 0x40
    int mSkeletonIndex; // 0x44
    FreestyleMove mFreestyleMove[4]; // 0x48
    int mCurrentMove; // 0xb8
    RndTex *mOutputTex; // 0xbc
    FreestyleMoveFrame *mDancerTakeFrames; // 0xc0
    int mDancerTakeFrameCount; // 0xc4
    int mRecordedFramesThisMove; // 0xc8
    std::vector<JointAngle> mJointAngles; // 0xcc
    std::vector<SkeletonJoint> mDispJoints; // 0xd8
    FreestyleFrameScores mFrameScoreSlots[2]; // 0xe4
    std::vector<JointPos> mJointPos; // 0x104
};
