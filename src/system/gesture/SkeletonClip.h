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
    void MakeSkeletonFrame(SkeletonFrame &, int) const;

    int unk0;
    int unk4;
    Vector3 unk8;
    Vector4 unk18; // could also be an XMVECTOR?
    bool unk28;
    Vector3 unk2c[kNumJoints];
    int unk16c[kNumJoints];
    int unk1bc;
    int unk1c0;
    float unk1c4;
};

class SkeletonClip : public CameraInput,
                     public RndAnimatable,
                     public RndPollable,
                     public SkeletonHistory {
public:
    struct MoveRating {
        String mName; // 0x0
        Symbol mExpected; // 0x8
        int unkc; // 0xc
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
    // CameraInput
protected:
    virtual const SkeletonFrame *PollNewFrame();

public:
    virtual const SkeletonHistory *History() const { return this; }
    // RndAnimatable
    virtual void StartAnim();
    virtual void SetFrame(float frame, float blend);
    virtual float EndFrame();
    // RndPollable
    virtual void Poll();
    // SkeletonHistory
    virtual bool PrevSkeleton(const Skeleton &, int, ArchiveSkeleton &, int &) const;

    static String sRemapClipSearch;
    static String sRemapClipReplace;

    OBJ_MEM_OVERLOAD(0x3C);
    NEW_OBJ(SkeletonClip);

    void SetRecordedTime(const DateTime &dt) { mTimeRecorded = dt; }
    void SetSong(Symbol sym) { mSong = sym; }
    void SetDifficulty(Difficulty d) { mDifficulty = d; }
    void SetBuild(const String &str) { mBuild = str; }
    bool IsRecording() const;
    const String &Path() const;
    String DateTimeStr() const;
    const char *DifficultyStr() const;
    bool IsFailClip() const;
    void SetAutoplay(bool);
    void SetPath(const char *);
    void EnableAlternateRecord(int);
    int NumMoveRatings() const;
    void WriteClip(FileStream &);
    const MoveRating &GetMoveRating(int) const;
    void FlushMoveRecord(const char *);
    void StartXboxRecording(const char *);
    bool SkeletonFrameAt(float, SkeletonFrame &) const;
    void StopRecording();
    void LoadClipFromFile(String, SkeletonClip *);
    float SongStartSeconds() const;
    float SongEndSeconds() const;
    void LoadClip(bool);
    void FillMoveRatings();

    static void Init();
    static const RecordedFrame *
    RecordedFrameAt(const std::vector<RecordedFrame> &, float, int &, int &);

protected:
    SkeletonClip();

    void WriteClipHeader(FileStream &);
    void WriteClipFrame(FileStream &, const RecordedFrame &);
    void StopRecordingNoClear();
    const RecordedFrame *CurRecordedFrame(int &, int &) const;

    static void LoadFrame(BinStream &, RecordedFrame &, int);

    std::vector<RecordedFrame> *mRecordedFrames; // 0x11f0
    SkeletonFrame *unk11f4; // 0x11f4
    String *unk11f8; // 0x11f8
    int unk11fc; // 0x11fc
    DateTime mTimeRecorded; // 0x1200
    Symbol mSong; // 0x1208
    Difficulty mDifficulty; // 0x120c
    String mBuild; // 0x1210
    Symbol mDefaultRating; // 0x1218
    int mWeighted; // 0x121c
    int mOverrideDiff; // 0x1220
    std::vector<MoveRating> mMoveRatings; // 0x1224
    bool unk1230;
    bool unk1231;
    FileStream *mFileStream; // 0x1234
    String mFile; // 0x1238
    int unk1240;
    bool mAutoplay; // 0x1244
};

void ReserveFrames();
