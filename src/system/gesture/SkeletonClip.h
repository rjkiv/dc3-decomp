#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "hamobj/Difficulty.h"
#include "math/Vec.h"
#include "os/DateTime.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "gesture/SkeletonHistory.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/MemMgr.h"

// size 0x1c8
struct RecordedFrame {
    void MakeSkeletonFrame(SkeletonFrame &cfd, int skel_idx) const;

    int mFrameNumber; // 0x0
    int mElapsedMs; // 0x4
    Vector3 mUpVector; // 0x8
    Plane mFloorPlane; // 0x18
    bool mTracked; // 0x28
    Vector3 mJointPositions[kNumJoints]; // 0x2c
    JointConfidence mJointConfidences[kNumJoints]; // 0x16c
    int mQualityFlags; // 0x1bc
    int mTrackingID; // 0x1c0
    float mSeconds; // 0x1c4
};

enum SkeletonClipWeighting {
    kClipNotWeighted = 0x0000,
    kClipWeighted = 0x0001,
    kClipDefaultWeighting = 0x0002,
};

class SkeletonClip : public CameraInput,
                     public RndAnimatable,
                     public RndPollable,
                     public SkeletonHistory {
public:
    struct MoveRating {
        String mMoveName; // 0x0
        Symbol mExpectedRating; // 0x8
        SkeletonClipWeighting mWeighting; // 0xc
    };
    // Hmx::Object
    virtual ~SkeletonClip();
    OBJ_CLASSNAME(SkeletonClip);
    OBJ_SET_TYPE(SkeletonClip);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

public:
    virtual const SkeletonHistory *History() const { return this; }
    // RndAnimatable
    virtual void StartAnim();
    virtual void SetFrame(float frame, float blend);
    virtual float EndFrame();
    // RndPollable
    virtual void Poll();
    // SkeletonHistory
    virtual bool PrevSkeleton(
        const Skeleton &current,
        int previous_ms,
        ArchiveSkeleton &previous,
        int &elapsed_ms
    ) const;

    static String sRemapClipSearch;
    static String sRemapClipReplace;

    OBJ_MEM_OVERLOAD(0x3C);
    NEW_OBJ(SkeletonClip);

    void SetRecordingPlayer(int player) { mRecordingPlayer = player; }
    bool IsRecording() const;
    const String &Path() const;
    String DateTimeStr() const;
    const char *DifficultyStr() const;
    bool IsFailClip() const;
    void SetAutoplay(bool autoplay);
    void SetPath(const char *file);
    void EnableAlternateRecord(int recordingBuffer);
    int NumMoveRatings() const;
    void WriteClip(FileStream &file_stream);
    const MoveRating &GetMoveRating(int bar) const;
    void FlushMoveRecord(const char *name);
    void StartXboxRecording(const char *path);
    bool SkeletonFrameAt(float song_seconds, SkeletonFrame &skel_frame) const;
    void StopRecording();
    void LoadClipFromFile(String file, SkeletonClip *clip);
    float SongStartSeconds() const;
    float SongEndSeconds() const;
    void LoadClip(bool tool_sync);
    void FillMoveRatings();
    void SwapMoveRecord();
    void PollRecording(const SkeletonFrame &frame_data);
    const char *SongStr() const;

    static void Init();
    static const RecordedFrame *RecordedFrameAt(
        const std::vector<RecordedFrame> &frames,
        float song_seconds,
        int &index,
        int &loop_count
    );

protected:
    SkeletonClip();

    virtual const SkeletonFrame *PollNewFrame();

    void WriteClipHeader(FileStream &f);
    void WriteClipFrame(FileStream &f, const RecordedFrame &rec_frame);
    void StopRecordingNoClear();
    const RecordedFrame *CurRecordedFrame(int &index, int &loop_count) const;

    static void LoadFrame(BinStream &s, RecordedFrame &rf, int clip_rev);

    std::vector<RecordedFrame> *mFrames; // 0x11f0
    SkeletonFrame *mCamFrame; // 0x11f4
    String *mLoadedFile; // 0x11f8
    int mRecordingPlayer; // 0x11fc
    DateTime mRecordDateTime; // 0x1200
    Symbol mRecordSong; // 0x1208
    Difficulty mRecordDifficulty; // 0x120c
    String mRecordBuild; // 0x1210
    Symbol mDefaultExpectedRating; // 0x1218
    SkeletonClipWeighting mWeighting; // 0x121c
    Difficulty mOverrideDifficulty; // 0x1220
    std::vector<MoveRating> mMoveRatings; // 0x1224
    bool mSuspended; // 0x1230
    bool mRecording; // 0x1231
    FileStream *mFileStream; // 0x1234
    String mFile; // 0x1238
    int mInternalFrame; // 0x1240
    bool mAutoplay; // 0x1244
};

void ReserveFrames();
