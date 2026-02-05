#include "RhythmDetector.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "ui/UIPanel.h"
#include "obj/DataFunc.h"
#include "gesture/SkeletonUpdate.h"

namespace {
    int kAnalyzeJoints[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                             10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    int gDebugBone = -1;
    float gAdjust = 1;
    int gLog = -1;
    bool gClamp = true;
    const char *kConv[] = { "------00++++++00----",
                            "---0+++0-----0+++0--",
                            "-++0--++0--++0--++0-",
                            "-0++--0++--0++--0++-" };
    int kConvCount = 4;
    int kConvLen;

    //     void AnalyzeData(
    //         const std::vector<RhythmDetector::Frame> &frames,
    //         float &f1,
    //         float &f2,
    //         float &f3,
    //         float f4,
    //         bool b1,
    //         Symbol sym,
    //         bool b2,
    //         DebugGraph *dbg,
    //         int i1,
    //         TextStream *stream
    //     ) {
    //         return;
    //     }

    DataNode TightenDebugBone(DataArray *da) {
        gAdjust *= 1.01f;
        MILO_LOG("scalar %f\n", gAdjust);
        return 0;
    }

    DataNode LoosenDebugBone(DataArray *da) {
        gAdjust *= 0.990099f;
        MILO_LOG("scalar %f\n", gAdjust);
        return 0;
    }

    DataNode DataSpaceCheat(DataArray *da) {
        gLog += 1;
        if (60 <= gLog) {
            gLog = -1;
        }
        return 0;
    }

    DataNode CycleDebugBone(DataArray *da) {
        gDebugBone += 1;
        gAdjust = 1.0;

        if (gDebugBone == 20) {
            gDebugBone = -1;
        }
        MILO_LOG("debug bone %d\n", gDebugBone);
        return 0;
    }

    void initCheat() {
        // if(SomeGlobalOrSymbol == 0) {
        // SomeGlobalOrSymbol = 1;
        Symbol cycle_movement_bone("cycle_movement_bone");
        DataRegisterFunc(cycle_movement_bone, CycleDebugBone);
        Symbol tighten_current_bone("tighten_current_bone");
        DataRegisterFunc(tighten_current_bone, TightenDebugBone);
        Symbol loosen_current_bone("loosen_current_bone");
        DataRegisterFunc(loosen_current_bone, LoosenDebugBone);
        Symbol ktb_debug_cheat("ktb_debug_cheat");
        DataRegisterFunc(ktb_debug_cheat, DataSpaceCheat);
        //}
    }

    float Mean(const std::vector<float> &vec, int start, int end) {
        // some fruity branchless stuff going on in here
        int size = (vec.size());
        end = size < end ? size : end;
        start = start < 0 ? 0 : start;
        float sum = 0.0f;
        for (int i = start; i < end; i++) {
            sum += vec[i];
        }
        int count = end - start;
        if (count == 0) {
            return 0.0f;
        }
        return sum / count;
    }

    float Variance(const std::vector<float> &vec, float mean, int start, int end) {
        // dear god please someone figure what isnt right here
        int size = (vec.size());
        end = size < end ? size : end;
        start = start < 0 ? 0 : start;
        float sum = 0.0f;
        for (int i = start; i < end; i++) {
            sum += (vec[i] - mean) * (vec[i] - mean);
        }
        int count = end - start;
        if (count != 0) {
            return sum / count;
        }
        return 0.0f;
    }

}

void EraseNewerData(std::vector<RhythmDetector::Frame> &vec, float time) {
    FOREACH (it, vec) {
        if (it->unk0 >= time) {
            vec.erase(it, vec.end());
            break;
        }
    }
}

void CameraToScreenUnit(Vector3 &vec, const Skeleton &skeleton, SkeletonJoint joint) {
    Vector2 skelPos;
    skeleton.ScreenPos(joint, skelPos);
    float y = -skeleton.TrackedJoints()[joint].unk60.z;
    vec.Set((skelPos.x - 0.5f) * 2.0f, y * 0.22977939f, (0.5f - skelPos.y) * 2.0f);
}

RhythmDetector::RhythmDetector()
    : mTracked(false), mRecording(0), mSkeletonID(-1), mBeats(8), mGroove(0),
      mRhythmDecay(0), mFold(2), mToleranceFactor(1.5f), mDirection(0, 0, 0), unk68(0),
      mDebugGraphA(0), mDebugGraphB(0), mDebugGraphC(0), mDebugGraphD(0), mDebugGraphE(0),
      unk80(0), unkaa4(0) {
    unk1c.mJointVelocities.clear();
    for (int i = 0; i < 8; i++) {
        unka84[i] = -1;
    }
    initCheat();
}

RhythmDetector::~RhythmDetector() {
    delete mDebugGraphA;
    delete mDebugGraphB;
    if (SkeletonUpdate::InstanceHandle().HasCallback(this)) {
        SkeletonUpdate::InstanceHandle().RemoveCallback(this);
    }
}

BEGIN_HANDLERS(RhythmDetector)
    HANDLE_ACTION(start_recording, StartRecording())
    HANDLE_ACTION(stop_recording, StopRecording())
    HANDLE_EXPR(is_recording, IsRecording())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RhythmDetector)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_PROP(rhythm_rating, mGroove);
    SYNC_PROP(rhythm_decay, mRhythmDecay)
    SYNC_PROP(num_beats_to_cover, mBeats)
    SYNC_PROP(beat_fold, mFold)
    SYNC_PROP(tolerance_factor, mToleranceFactor)
    SYNC_PROP(dir_x, mDirection.x)
    SYNC_PROP(dir_y, mDirection.y)
    SYNC_PROP(dir_z, mDirection.z)
END_PROPSYNCS

BEGIN_SAVES(RhythmDetector)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(RndPollable)
    bs << mBeats;
    bs << mFold;
    bs << mToleranceFactor;
    bs << mDirection.x;
    bs << mDirection.y;
    bs << mDirection.z;
END_SAVES

BEGIN_COPYS(RhythmDetector)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RhythmDetector)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBeats)
        COPY_MEMBER(mFold)
        COPY_MEMBER(mToleranceFactor)
        COPY_MEMBER(mDirection)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RhythmDetector)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(RndPollable)
    if (d.rev >= 1) {
        d >> mBeats;
    }
    if (d.rev >= 2) {
        d >> mFold;
        d >> mToleranceFactor;
        d >> mDirection.x;
        d >> mDirection.y;
        d >> mDirection.z;
        Normalize(mDirection, mDirection);
    }
END_LOADS

void RhythmDetector::Poll() {
    if (mRecording) {
        if (TheGestureMgr->GetSkeleton(mSkeletonID).IsTracked()) {
            if (mDebugGraphA)
                mDebugGraphA->Draw();
            if (mDebugGraphB)
                mDebugGraphB->Draw();
            if (mDebugGraphC)
                mDebugGraphC->Draw();
            if (mDebugGraphD)
                mDebugGraphD->Draw();
            if (mDebugGraphE)
                mDebugGraphE->Draw();
        }
    }
}

void RhythmDetector::Enter() {
    RndPollable::Enter();
    if (SkeletonUpdate::HasInstance()
        && !SkeletonUpdate::InstanceHandle().HasCallback(this)) {
        SkeletonUpdate::InstanceHandle().AddCallback(this);
    }
}

void RhythmDetector::PostUpdate(const SkeletonUpdateData *data) {
    if (mSkeletonID >= 0) {
        mTracked = TheGestureMgr->GetSkeleton(mSkeletonID).IsTracked();
    } else {
        mTracked = false;
    }
    if (mRecording == 0 || !mTracked) {
        unk1c.mJointVelocities.clear();
        unk38.clear();
        unk2c.clear();
        mRecordData.frames.clear();
    } else if (data) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(mSkeletonID);
        if (skeleton.ElapsedMs() != data->unk8->mElapsedMs) {
            MILO_WARN("current skeleton doesn't match update data");
        }
        AddFrame(skeleton);
        for (int i = 0; i < kNumJoints; i++) {
            CameraToScreenUnit(unkaac[i], skeleton, (SkeletonJoint)i);
        }
        ProcessFrames();
    }
}

float RhythmDetector::Groove() const {
    if (mTracked)
        return mGroove;
    else
        return 0;
}

float RhythmDetector::Freshness() const {
    if (mTracked)
        return 1 - mRhythmDecay;
    else
        return 0;
}
Vector4 RhythmDetector::Data1(int idx) const {
    const Vector3 &v = unkaac[idx];
    return Vector4(v.x, v.y, v.z, 0);
}

Vector4 RhythmDetector::Data2(int) const { return Vector4(0, 1, 1, 1); }

void RhythmDetector::RemoveDebugGraphs() {
    RELEASE(mDebugGraphA);
    RELEASE(mDebugGraphB);
    RELEASE(mDebugGraphC);
    RELEASE(mDebugGraphD);
    RELEASE(mDebugGraphE);
}

void RhythmDetector::AddDebugGraph(
    float f1, float f2, float f3, float f4, Hmx::Color color
) {
    delete mDebugGraphE;
    mDebugGraphE = new DebugGraph(
        f1,
        f2,
        f3,
        f4,
        color,
        Hmx::Color(0.4f, 0.4f, 0.4f, 0.8f),
        120,
        0,
        2,
        MakeString(
            "beats %d  fold %d  dir %.1f %.1f %.1f",
            (int)mBeats,
            mFold,
            mDirection.x,
            mDirection.y,
            mDirection.z
        )
    );
    mDebugGraphE->SetUnk44(1);
}

void RhythmDetector::AddFullDebugGraphs() {
    if (gLog != -1) {
        Hmx::Color red(1, 0, 0, 1);
        delete mDebugGraphA;
        mDebugGraphA = new DebugGraph(
            0.1f, 0.0f, 0.8f, 0.06f, red, Hmx::Color(0, 0, 0, 0), 120, -1.1f, 1.1f, ""
        );
        mDebugGraphA->SetUnk50(false);
    }
}

void RhythmDetector::StartRecording() {
    if (++mRecording == 1) {
        AddFullDebugGraphs();
        unkaa8 = TheTaskMgr.Beat();
        ClearData();
    }
    MILO_ASSERT(mRecording >= 1, 0x3cc);
    MILO_ASSERT(mRecording <= 2, 0x3cd);
}

void RhythmDetector::StopRecording() {
    if (--mRecording == 0) {
        unkaa8 = TheTaskMgr.Beat();
        ClearData();
    }
    MILO_ASSERT(mRecording >= 0, 0x3da);
    MILO_ASSERT(mRecording <= 1, 0x3db);
}

void RhythmDetector::ClearData() {
    unk68 = 0;
    unk2c.clear();
    unk38.clear();
    mRecordData.frames.clear();
    mGroove = 0;
    mRhythmDecay = 0;
    mRecordData.unk18 = true;
    mRecordData.unk8 = -1;
    mRecordData.unkc = -1;
    mRecordData.unk0 = -1;
    mRecordData.unk4 = -1;
    mRecordData.unk10 = -1;
    mRecordData.unk14 = -1;
    unk14.clear();
    AddFullDebugGraphs();
}

// void RhythmDetector::AddFrame(BaseSkeleton const &skel) {
//     auto panel = ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
//     if (panel) {
//         Vector3 jointPosVec;
//         for (int skelJoint = 0; skelJoint < kNumJoints; skelJoint++) {
//             skel.JointPos(
//                 kCoordCamera, static_cast<SkeletonJoint>(skelJoint), jointPosVec
//             );
//         }
//         float beat = TheTaskMgr.Beat();
//         float seconds = TheTaskMgr.Seconds(TaskMgr::kRealTime);
//         float beatDiff = beat - unkaa8;
//         if (beatDiff < 0.0) {
//             ClearData();
//             beatDiff = 0.0;
//         }
//         int temp = -1;
//         int anotherInt = 0;
//         float something = 0.0;
//         for (int i = 0; i < 8; i++) {
//             if (0.0 < unka80.back()
//                 && ((fabs(seconds - unka80.back()) - 0.1) < something || temp == -1)) {
//                 something = fabs(seconds - unka80.back()) - 0.1;
//                 temp = anotherInt;
//                 anotherInt++;
//             }
//         }
//         if (temp != -1) {
//         }
//     }
// }

// const RhythmDetector::RecordData &
// RhythmDetector::GetRecord(float f1, float f2, bool b, Symbol sym, TextStream *stream) {
//     RecordData ret = mRecordData;
//     if (mRecordData.unkbec == f1 && mRecordData.unkbf8 == f2) {
//         if (stream) {
//             AnalyzeData(
//                 unk38,
//                 mRecordData.unkbfc,
//                 mRecordData.mTracked00,
//                 mRhythmDecay,
//                 mToleranceFactor,
//                 mDebugGraphA->GetmFold(),
//                 0,
//                 b,
//                 0,
//                 mDebugGraphA->GetUnk38(),
//                 stream
//             );
//             mTracked04 = true;
//         }
//     } else {
//         if (mTracked04 == false) {
//             MILO_NOTIFY(
//                 "new rhythm detector window w/o finalization [%.1f,%.1f] to [%.1f,
//                 %.1f]", mRecordData.unkbec, mRecordData.unkbf0, f1, f2
//             );
//         }
//         ClearData();
//         mRecordData.unkbf0 = f2;
//         mTracked04 = false;
//         mRecordData.unkbf4 = -1.0;
//         mRecordData.unkbf8 = -1.0;
//         mTracked08.clear();
//     }
//     return ret;
// }
