#include "hamobj/FreestyleMoveRecorder.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraInput.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/FreestyleMove.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "stl/_vector.h"
#include "utl/FileStream.h"
#include "utl/Symbol.h"

static FreestyleMoveRecorder *sStaticFreestyleMoveRecorder;
static DancerSkeleton sLastComparedDancerSkel;

FreestyleMoveRecorder::FreestyleMoveRecorder()
    : mFakeSkeletonTime(0), mFakeSkeletonFrames(0), mFakeSkeletonFrameCount(0),
      mRecordedAttemptFrames(0), mDebugAlreadyLoadedMoveFor(-1), mMaxFrames(60),
      mRecordTime(-1), mPlayTime(-1), mFrameRate(15), mStopRecordingAt(-1),
      mRecordingDancerTake(0), mPlayPreviousRecording(0), mSkeletonIndex(-1),
      mCurrentMove(0) {
    mOutputTex = Hmx::Object::New<RndTex>();
    mOutputTex->SetBitmap(320, 240, 16, RndTex::kRegularLinear, false, nullptr);
    mJointAngles.push_back(JointAngle(kJointHandRight));
    mJointAngles.push_back(JointAngle(kJointHandLeft));
    mJointAngles.push_back(JointAngle(kJointAnkleRight));
    mJointAngles.push_back(JointAngle(kJointAnkleLeft));
    mJointAngles.push_back(JointAngle(kJointKneeRight));
    mJointAngles.push_back(JointAngle(kJointKneeLeft));
    mDispJoints.push_back(kJointHandRight);
    mDispJoints.push_back(kJointHandLeft);
    mDispJoints.push_back(kJointAnkleRight);
    mDispJoints.push_back(kJointAnkleLeft);
    mDispJoints.push_back(kJointHead);
    mDispJoints.push_back(kJointHipCenter);
    mJointPos.push_back(JointPos(kJointHandRight, kCoordRightArm));
    mJointPos.push_back(JointPos(kJointHandLeft, kCoordLeftArm));
    mJointPos.push_back(JointPos(kJointElbowRight, kCoordRightArm));
    mJointPos.push_back(JointPos(kJointElbowLeft, kCoordLeftArm));
    mJointPos.push_back(JointPos(kJointAnkleRight, kCoordRightLeg));
    mJointPos.push_back(JointPos(kJointAnkleLeft, kCoordLeftLeg));
    mJointPos.push_back(JointPos(kJointKneeRight, kCoordRightLeg));
    mJointPos.push_back(JointPos(kJointKneeLeft, kCoordLeftLeg));
    mDancerTakeFrames = new FreestyleMoveFrame[mMaxFrames];
    DataRegisterFunc("bam_record_attempt", OnRecordAttempt);
    DataRegisterFunc("bam_write_created", OnWriteCreated);
    DataRegisterFunc("bam_read_created", OnReadCreated);
    DataRegisterFunc("bam_read_attempt", OnReadAttempt);
    DataRegisterFunc("bam_clear", OnClearAttempt);
}

FreestyleMoveRecorder::~FreestyleMoveRecorder() {
    delete mOutputTex;
    delete[] mDancerTakeFrames;
    delete[] mRecordedAttemptFrames;
    delete[] mFakeSkeletonFrames;
}

void FreestyleMoveRecorder::Free() {
    mRecordTime = -1;
    mPlayTime = -1;
    for (int i = 4; i != 0; i--) {
        mFreestyleMove[mCurrentMove].Free();
    }
}

void FreestyleMoveRecorder::StartRecording() {
    mStopRecordingAt = -1;
    mRecordingDancerTake = false;
    mRecordTime = 0;
    mPlayTime = -1;
    if (mDebugAlreadyLoadedMoveFor != mCurrentMove) {
        mFreestyleMove[mCurrentMove].Init(mMaxFrames);
    }
}

void FreestyleMoveRecorder::ClearRecording() {
    if (mDebugAlreadyLoadedMoveFor != mCurrentMove) {
        mFreestyleMove[mCurrentMove].Clear();
    }
    mRecordedFramesThisMove = 0;
}

void FreestyleMoveRecorder::StartRecordingDancerTake() {
    StartRecording();
    mRecordingDancerTake = true;
}

void FreestyleMoveRecorder::StartPlayback(bool param_1) {
    mPlayPreviousRecording = param_1;
    mPlayTime = 0;
}

void FreestyleMoveRecorder::StopPlayback() { mPlayTime = -1; }

void FreestyleMoveRecorder::ClearDancerTake() { mDancerTakeFrameCount = 0; }

void FreestyleMoveRecorder::AssignStaticInstance() {
    sStaticFreestyleMoveRecorder = this;
}

void FreestyleMoveRecorder::UpdateRecordingAttempt(
    const BaseSkeleton *skeleton, float f2
) {
    if (mRecordingName != gNullStr) {
        mRecordedAttemptFrames[mRecordedAttemptFrameCount].mSkeleton.Set(*skeleton);
        mRecordedAttemptFrames[mRecordedAttemptFrameCount].mTime = f2;
        mRecordedAttemptFrameCount++;
    }
}

void FreestyleMoveRecorder::RecordMoveAttempt(String str) {
    mRecordingName = str;
    delete[] mRecordedAttemptFrames;
    mRecordedAttemptFrames = new FreestyleMoveFrame[480];
    mRecordedAttemptFrameCount = 0;
}

void FreestyleMoveRecorder::WriteRecordedMoveAttempt() {
    WriteFreestyleMoveClip(
        mRecordingName, mRecordedAttemptFrameCount, mRecordedAttemptFrames
    );
    mRecordingName = gNullStr;
    delete[] mRecordedAttemptFrames;
    mRecordedAttemptFrames = nullptr;
    mRecordedAttemptFrameCount = 0;
}

void FreestyleMoveRecorder::ClearFreestyleMoveClip() {
    delete[] mFakeSkeletonFrames;
    mFakeSkeletonFrames = nullptr;
    mFakeSkeletonFrameCount = 0;
}

void FreestyleMoveRecorder::PlaybackComplete() {
    if (mRecordingName != gNullStr) {
        WriteRecordedMoveAttempt();
    }
}

void FreestyleMoveRecorder::ClearFrameScores() {
    for (int i = 0; i < 2; i++) {
        mFrameScoreSlots[i].Clear();
    }
}

void FreestyleMoveRecorder::WriteFreestyleMoveClip(
    String str, int framecount, FreestyleMoveFrame *frames
) {
    if (str.length() > 0x26) {
        str.resize(0x26);
    }
    str += ".bamclp";
    const char *path = MakeString("devkit:\\%s", str);
    FileStream stream(path, FileStream::kWrite, true);
    stream << mSongName;
    stream << framecount;
    for (int i = 0; i < framecount; i++) {
        frames[i].mSkeleton.Write(stream);
        stream << frames[i].mTime;
    }
    MILO_LOG("Saved clip to %s, framecount: %d\n", path, framecount);
}

void FreestyleMoveRecorder::ReadFreestyleMoveClip(
    String str, int &framecount, FreestyleMoveFrame *frames
) {
    if (str.length() > 0x26) {
        str.resize(0x26);
    }
    str += ".bamclp";
    const char *path = MakeString("devkit:\\%s", str);
    FileStream stream(path, FileStream::kRead, true);
    Symbol s;
    stream >> s;
    stream >> framecount;
    for (int i = 0; i < framecount; i++) {
        frames[i].mSkeleton.Read(stream);
        stream >> frames[i].mTime;
    }
    MILO_LOG("Loaded clip that was recorded with %s, framecount: %d\n", s, framecount);
}

DataNode FreestyleMoveRecorder::OnRecordAttempt(DataArray *a) {
    String str;
    if (a->Size() >= 2) {
        str = a->Str(1);
    } else {
        str = sStaticFreestyleMoveRecorder->mSongName.Str();
        str += "_attempt_";
        DateTime dt;
        GetDateAndTime(dt);
        str += MakeString("%02d%02d_%02d%02d", dt.Month(), dt.mDay, dt.mHour, dt.mMin);
    }
    sStaticFreestyleMoveRecorder->RecordMoveAttempt(str);
    return 0;
}

DataNode FreestyleMoveRecorder::OnWriteCreated(DataArray *a) {
    String str;
    if (a->Size() >= 2) {
        str = a->Str(1);
    } else {
        str = sStaticFreestyleMoveRecorder->mSongName.Str();
        str += "_created_";
        DateTime dt;
        GetDateAndTime(dt);
        str += MakeString("%02d%02d_%02d%02d", dt.Month(), dt.mDay, dt.mHour, dt.mMin);
    }
    sStaticFreestyleMoveRecorder->WriteFreestyleMoveClip(
        str,
        sStaticFreestyleMoveRecorder
            ->mFreestyleMove[sStaticFreestyleMoveRecorder->mCurrentMove]
            .mFrameCount,
        sStaticFreestyleMoveRecorder
            ->mFreestyleMove[sStaticFreestyleMoveRecorder->mCurrentMove]
            .mSkeletonFrames
    );
    return 0;
}

DataNode FreestyleMoveRecorder::OnReadCreated(DataArray *a) {
    int framecount;
    sStaticFreestyleMoveRecorder->ReadFreestyleMoveClip(
        a->Str(1),
        framecount,
        sStaticFreestyleMoveRecorder
            ->mFreestyleMove[sStaticFreestyleMoveRecorder->mCurrentMove]
            .mSkeletonFrames
    );
    sStaticFreestyleMoveRecorder
        ->mFreestyleMove[sStaticFreestyleMoveRecorder->mCurrentMove]
        .Init(sStaticFreestyleMoveRecorder->mMaxFrames);
    sStaticFreestyleMoveRecorder
        ->mFreestyleMove[sStaticFreestyleMoveRecorder->mCurrentMove]
        .mFrameCount = framecount;
    sStaticFreestyleMoveRecorder->mDebugAlreadyLoadedMoveFor =
        sStaticFreestyleMoveRecorder->mCurrentMove;
    return 0;
}

DataNode FreestyleMoveRecorder::OnReadAttempt(DataArray *a) {
    delete[] sStaticFreestyleMoveRecorder->mFakeSkeletonFrames;
    sStaticFreestyleMoveRecorder->mFakeSkeletonFrames = new FreestyleMoveFrame[480];
    sStaticFreestyleMoveRecorder->ReadFreestyleMoveClip(
        a->Str(1),
        sStaticFreestyleMoveRecorder->mFakeSkeletonFrameCount,
        sStaticFreestyleMoveRecorder->mFakeSkeletonFrames
    );
    return 0;
}

DataNode FreestyleMoveRecorder::OnClearAttempt(DataArray *a) {
    sStaticFreestyleMoveRecorder->ClearFreestyleMoveClip();
    return 0;
}

void FreestyleMoveRecorder::StopRecording() {
    mStopRecordingAt = mFreestyleMove[mCurrentMove].mFrameCount + 2;
}

void FreestyleMoveRecorder::DrawDebug() {
    static float top = 0.3f;
    static float left = 0.1f;
    static float width = 0.3f;
    static SkeletonViz *vizLive;
    static SkeletonViz *vizRecorded;
    if (DataVariable("bam_debug").Int() != 0) {
        if (!vizRecorded) {
            vizRecorded = Hmx::Object::New<SkeletonViz>();
            vizRecorded->Init();
            vizLive = Hmx::Object::New<SkeletonViz>();
            vizLive->Init();
        }

        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        std::vector<SkeletonCallback *> callbackList;
        callbackList.push_back(this);

        Hmx::Rect recordedRect(left, top, width, width / TheRnd.YRatio());
        Hmx::Color color(0, 0, 0, 0.4f);
        TheRnd.DrawRectScreen(recordedRect, color, nullptr, nullptr, nullptr);
        vizRecorded->SetUsePhysicalCam(true);
        vizRecorded->SetPhysicalCamScreenRect(recordedRect);
        vizRecorded->Visualize(
            *handle.GetCameraInput(), sLastComparedDancerSkel, &callbackList, false
        );

        Hmx::Rect liveRect(left + width + 0.1f, top, width, width / TheRnd.YRatio());
        Hmx::Color color2(0, 0, 0, 0.4f);
        TheRnd.DrawRectScreen(liveRect, color2, nullptr, nullptr, nullptr);
        vizLive->SetUsePhysicalCam(true);
        vizLive->SetPhysicalCamScreenRect(liveRect);
        BaseSkeleton *liveSkeleton = GetLiveSkeleton();
        if (liveSkeleton) {
            vizLive->Visualize(
                *handle.GetCameraInput(), *liveSkeleton, &callbackList, false
            );
        }
    }
}

void FreestyleMoveRecorder::UpdateFakeSkeleton() {
    static int sLastBeat;
    mFakeSkeletonTime += TheTaskMgr.DeltaUISeconds();
    int beat = (int)TheTaskMgr.Beat() % 4;
    if (beat == 0 && sLastBeat != 0) {
        mFakeSkeletonTime = 0;
    }
    sLastBeat = beat;
}

BaseSkeleton *FreestyleMoveRecorder::GetLiveSkeleton() {
    if (mFakeSkeletonFrameCount > 0) {
        int i3 = 0;
        int i5 = 0;
        for (; i5 < mFakeSkeletonFrameCount && i3 != mPlaybackRep; i5++) {
            if (mFakeSkeletonFrames[i5].mTime > mFakeSkeletonFrames[i5 + 1].mTime) {
                i3++;
            }
        }
        for (; i5 < mFakeSkeletonFrameCount; i5++) {
            if (mFakeSkeletonFrames[i5].mTime > mFakeSkeletonTime * 1000) {
                break;
            }
        }
        return &mFakeSkeletonFrames[i5].mSkeleton;
    } else {
        return mSkeletonIndex >= 0 ? &TheGestureMgr->GetSkeleton(mSkeletonIndex)
                                   : nullptr;
    }
}

void FreestyleMoveRecorder::CompareDisplacementVectors(
    const Vector3 &v1, int ms1, const Vector3 &v2, int ms2, float &score, float &weight
) const {
    float len1 = Length(v1);
    float len2 = Length(v2);

    weight = Min<float>(ms2 ? len2 / (float)ms2 : 0.0f, ms1 ? len1 / (float)ms1 : 0.0f)
        + 1e-5f;

    Vector3 scale1;
    Scale(v1, len1 > 0 ? 1 / len1 : 0.0f, scale1);
    Vector3 scale2;
    Scale(v2, len2 > 0 ? 1 / len2 : 0.0f, scale2);
    float clamped = Clamp(0.0f, 1.0f, -(Dot(scale1, scale2) * 0.87f - 1.0f));
    score = 1 - Clamp(0.0f, 1.0f, clamped * clamped * 20.0f);
}
