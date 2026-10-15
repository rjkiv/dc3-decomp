#include "RhythmDetector.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "math/Vec.inl"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "stl/_vector.h"
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
    int kConvLen = strlen(kConv[0]);

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
        gAdjust = 1;

        if (gDebugBone == 20) {
            gDebugBone = -1;
        }
        MILO_LOG("debug bone %d\n", gDebugBone);
        return 0;
    }

    void initCheat() {
        static bool sInitted = false;
        if (!sInitted) {
            sInitted = true;
            DataRegisterFunc("cycle_movement_bone", CycleDebugBone);
            DataRegisterFunc("tighten_current_bone", TightenDebugBone);
            DataRegisterFunc("loosen_current_bone", LoosenDebugBone);
            DataRegisterFunc("ktb_debug_cheat", DataSpaceCheat);
        }
    }

    float Mean(const std::vector<float> &vec, int start, int end) {
        int size = (vec.size());
        end = size < end ? size : end;
        start = start > 0 ? start : 0;
        float sum = 0.0f;
        for (int i = start; i < end; i++) {
            sum += vec[i];
        }
        int count = end - start;
        if (count != 0) {
            return sum / count;
        } else {
            return 0.0f;
        }
    }

    float Variance(const std::vector<float> &vec, float mean, int start, int end) {
        int size = (vec.size());
        end = size < end ? size : end;
        start = start > 0 ? start : 0;
        float sum = 0.0f;
        for (int i = start; i < end; i++) {
            sum += (vec[i] - mean) * (vec[i] - mean);
        }
        int count = end - start;
        if (count != 0) {
            return sum / count;
        } else {
            return 0.0f;
        }
    }

    const std::vector<float> &minJointSpeedVector() {
        static std::vector<float> data;
        static UIPanel *rhythm_detector_panel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);

        DataArray *typeDef = rhythm_detector_panel->TypeDef();
        if (data.empty()) {
            DataArray *minJoints = typeDef->FindArray("min_joint_speed");
            MILO_ASSERT(minJoints->Size() == kNumJoints + 1, 0xE4);
            for (int i = 1; i < minJoints->Size(); i++) {
                data.push_back(minJoints->Float(i));
            }
            MILO_ASSERT(data.size() == kNumJoints, 0xF2);
        }
        return data;
    }

    const std::vector<float> &jointWeight() {
        static std::vector<float> data;
        static UIPanel *rhythm_detector_panel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);

        DataArray *typeDef = rhythm_detector_panel->TypeDef();
        if (data.empty()) {
            DataArray *minJoints = typeDef->FindArray("joint_weight");
            MILO_ASSERT(minJoints->Size() == kNumJoints + 1, 0x104);
            for (int i = 1; i < minJoints->Size(); i++) {
                data.push_back(minJoints->Float(i));
            }
            MILO_ASSERT(data.size() == kNumJoints, 0x112);
        }
        return data;
    }

    void AnalyzeData(
        const std::vector<RhythmDetector::Frame> &frames,
        float &f1,
        float &f2,
        float &f3,
        float f4,
        bool b1,
        Symbol sym,
        bool b2,
        DebugGraph *dbg,
        int i1,
        TextStream *stream
    ) {
        float f21 = (float)frames.size() * 0.025f;
        float f24 = 0;
        float f23 = 0;
        if (frames.size() >= 10) {
            for (int i = 0; i < kNumJoints; i++) {
                float curWeight = jointWeight()[i];
                if (curWeight != 0) {
                    if (stream) {
                        *stream << "<br>";
                    }
                    for (int j = 0; j < 3; j++) {
                        float f26 = 0;
                        float f27 = 0;
                        static std::vector<float> raw;
                        static std::vector<float> normalized;

                        raw.resize(Min<unsigned int>(40, frames.size()));
                        normalized.resize(Min<unsigned int>(40, frames.size()));

                        for (int k = 0; k < raw.size(); k++) {
                            float val = frames[k].mJointVelocities[i][j];
                            val = fabsf(val);
                            raw[k] = val;
                            f26 += val;
                        }
                        {
                            float mean = Mean(raw, 0, 10);
                            float variance = Variance(raw, mean, 0, 10);
                            for (int k = 0; k < 6; k++) {
                                normalized[k] = (raw[k] - mean) * (1 / variance);
                            }
                        }
                        int nextIdx = Max<unsigned int>(6, raw.size() - 6);
                        for (int k = 6; k < nextIdx; k++) {
                            int l = k - 5;
                            float f28 = Mean(raw, l, l + 10);
                            float f17 = raw[k] - f28;
                            f28 = Variance(raw, f28, l, l + 10);
                            normalized[k] = f17 / f28;
                        }
                        {
                            float mean = Mean(raw, nextIdx - 5, nextIdx + 5);
                            float variance =
                                Variance(raw, mean, nextIdx - 5, nextIdx + 5);
                            for (int k = nextIdx; k < raw.size() - 1; k++) {
                                normalized[k] = (raw[k] - mean) * (1 / variance);
                            }
                        }
                        float f17 = 0;
                        for (int k = 0; k < normalized.size(); k++) {
                            f17 += fabsf(normalized[k]);
                        }

                        for (int k = 0; k < kConvCount; k++) {
                            const char *curConv = kConv[k];
                            for (int l = 0; l < kConvLen; l++) {
                                float f18 = 0;
                                for (int n = 0; n < normalized.size(); n++) {
                                    float f16 = normalized[n];
                                    int convIdx = (l + n) % kConvLen;
                                    if (curConv[convIdx] == '-') {
                                        f16 *= -1;
                                    } else if (curConv[convIdx] == '0') {
                                        f16 = fabsf(f16);
                                    }
                                    f18 += f16;
                                }
                                if (f18 > f27) {
                                    f27 = f18;
                                }
                            }
                        }
                        float div = f27 / f17;
                        f27 = div * f26;
                        if (stream) {
                            *stream << " ";
                            *stream << MakeString("%0.2f", div);
                            *stream << "&middot;";
                            int x = f26 * curWeight;
                            *stream << x;
                            if (x < 10) {
                                *stream << "&nbsp;&nbsp;";
                            } else if (x < 100) {
                                *stream << "&nbsp;";
                            }
                        }

                        f23 += f26 * curWeight;
                        f24 += f27 * curWeight;
                    }
                }
            }

            float f19 = Max(f23, f21 * 200);
            f1 = (f24 / f19) * f21;
            f2 = f23 < 200 ? 0 : f23;
            if (stream) {
                *stream << "<br>";
                *stream << (int)f24;
                *stream << "/";
                *stream << (int)f23;
                *stream << "=";
                *stream << f1;
            }
        }
    }

}

void EraseNewerData(std::vector<RhythmDetector::Frame> &frames, float maxBeat) {
    auto found = frames.end();
    FOREACH (it, frames) {
        if (it->mBeat >= maxBeat) {
            found = it;
            break;
        }
    }
    frames.erase(found, frames.end());
}

void CameraToScreenUnit(Vector3 &out, const Skeleton &skel, SkeletonJoint joint) {
    Vector2 skelPos;
    skel.ScreenPos(joint, skelPos);
    float y = -skel.TrackedJoints()[joint].mRawCameraPos.z;
    out.Set((skelPos.x - 0.5f) * 2.0f, y * 0.22977939f, (0.5f - skelPos.y) * 2.0f);
}

void SetupFrame(
    RhythmDetector::Frame &f,
    float prev_beat,
    float delt_beat,
    const Vector3 *prev,
    const Vector3 *pos,
    float value
) {
    MILO_ASSERT(prev_beat >= 0, 0x4d7);
    MILO_ASSERT(delt_beat >= 0, 0x4d8);
    MILO_ASSERT(prev, 0x4d9);
    MILO_ASSERT(pos, 0x4da);

    static UIPanel *rhythmPanel =
        ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
    minJointSpeedVector();

    f.mJointVelocities.resize(kNumJoints);

    for (int i = 0; i < kNumJoints; i++) {
        int currentJoint = kAnalyzeJoints[i];
        const Vector3 &prevJoint = prev[currentJoint];
        const Vector3 &posJoint = pos[currentJoint];
        Vector3 vsub;
        Subtract(posJoint, prevJoint, vsub);
        f.mJointVelocities[i].x = vsub.x * (1 / value);
        f.mJointVelocities[i].y = vsub.y * (1 / value);
        f.mJointVelocities[i].z = vsub.z * (1 / value);
    }
    f.mBeat = prev_beat + delt_beat;
}

RhythmDetector::Frame BlendFrameDataToBeat(
    const RhythmDetector::Frame &a, const RhythmDetector::Frame &b, float f3
) {
    if (f3 < a.mBeat || b.mBeat < f3) {
        MILO_NOTIFY(
            "bad rhythm detector floating point precision at %f %f %f\n",
            a.mBeat,
            b.mBeat,
            f3
        );
    }
    const int kSize = a.mJointVelocities.size();
    MILO_ASSERT(kSize == b.mJointVelocities.size(), 0x5A9);

    float t = Clamp(0.0f, 1.0f, (f3 - a.mBeat) / (b.mBeat - a.mBeat));
    RhythmDetector::Frame out;
    out.mBeat = f3;
    out.mJointVelocities.resize(kSize);

    for (int i = 0; i < kSize; i++) {
        for (int j = 0; j < 3; j++) {
            out.mJointVelocities[i][j] =
                Interp(a.mJointVelocities[i][j], b.mJointVelocities[i][j], t);
        }
    }

    return out;
}

RhythmDetector::RhythmDetector()
    : mHasSkeleton(false), mRecording(0), mUseSkeletonIndex(-1), mNumBeatsToCover(8),
      mRhythmRating(0), mRhythmDecay(0), mBeatFold(2), mToleranceFactor(1.5f),
      mDir(0, 0, 0), mThreadTotalSkelBeat(0), mGraphVelX(0), mGraphFolded(0),
      mGraphStdDev(0), mGraphStdDevBeat(0), mGraphRating(0), mCurrIdx(0), unkaa4(0) {
    mLastFrame.mJointVelocities.clear();
    for (int i = 0; i < 8; i++) {
        unka84[i] = -1;
    }
    initCheat();
}

RhythmDetector::~RhythmDetector() {
    delete mGraphVelX;
    delete mGraphFolded;
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
    SYNC_PROP(rhythm_rating, mRhythmRating);
    SYNC_PROP(rhythm_decay, mRhythmDecay)
    SYNC_PROP(num_beats_to_cover, mNumBeatsToCover)
    SYNC_PROP(beat_fold, mBeatFold)
    SYNC_PROP(tolerance_factor, mToleranceFactor)
    SYNC_PROP(dir_x, mDir.x)
    SYNC_PROP(dir_y, mDir.y)
    SYNC_PROP(dir_z, mDir.z)
END_PROPSYNCS

BEGIN_SAVES(RhythmDetector)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(RndPollable)
    bs << mNumBeatsToCover;
    bs << mBeatFold;
    bs << mToleranceFactor;
    bs << mDir.x;
    bs << mDir.y;
    bs << mDir.z;
END_SAVES

BEGIN_COPYS(RhythmDetector)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RhythmDetector)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNumBeatsToCover)
        COPY_MEMBER(mBeatFold)
        COPY_MEMBER(mToleranceFactor)
        COPY_MEMBER(mDir)
    END_COPYING_MEMBERS
END_COPYS

// shows up in the binary, needs to go before the revs
const int kAnalyzeJointsButNotAnonymousLol[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                                 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

int dummyfuncinrhythmdetectorlmao(int idx) {
    return kAnalyzeJointsButNotAnonymousLol[idx];
}

INIT_REVS(2, 0)

BEGIN_LOADS(RhythmDetector)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(RndPollable)
    if (d.rev >= 1) {
        d >> mNumBeatsToCover;
    }
    if (d.rev >= 2) {
        d >> mBeatFold;
        d >> mToleranceFactor;
        d >> mDir.x;
        d >> mDir.y;
        d >> mDir.z;
        Normalize(mDir, mDir);
    }
END_LOADS

void RhythmDetector::Poll() {
    if (mRecording) {
        if (TheGestureMgr->GetSkeleton(mUseSkeletonIndex).IsTracked()) {
            if (mGraphVelX)
                mGraphVelX->Draw();
            if (mGraphFolded)
                mGraphFolded->Draw();
            if (mGraphStdDev)
                mGraphStdDev->Draw();
            if (mGraphStdDevBeat)
                mGraphStdDevBeat->Draw();
            if (mGraphRating)
                mGraphRating->Draw();
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
    if (mUseSkeletonIndex >= 0) {
        mHasSkeleton = TheGestureMgr->GetSkeleton(mUseSkeletonIndex).IsTracked();
    } else {
        mHasSkeleton = false;
    }
    if (mRecording == 0 || !mHasSkeleton) {
        mLastFrame.mJointVelocities.clear();
        unk38.clear();
        mFrameData.clear();
        mRecordData.mFrameData.clear();
    } else if (data) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(mUseSkeletonIndex);
        if (skeleton.ElapsedMs() != data->mFrame.mElapsedMs) {
            MILO_WARN("current skeleton doesn't match update data");
        }
        AddFrame(skeleton);
        for (int i = 0; i < kNumJoints; i++) {
            CameraToScreenUnit(mScreenJointPos[i], skeleton, (SkeletonJoint)i);
        }
        ProcessFrames();
    }
}

float RhythmDetector::Groove() const {
    if (mHasSkeleton)
        return mRhythmRating;
    else
        return 0;
}

float RhythmDetector::Freshness() const {
    if (mHasSkeleton)
        return 1 - mRhythmDecay;
    else
        return 0;
}

Vector4 RhythmDetector::Data1(int joint) const {
    const Vector3 &v = mScreenJointPos[joint];
    return Vector4(v.x, v.y, v.z, 0);
}

Vector4 RhythmDetector::Data2(int joint) const { return Vector4(0, 1, 1, 1); }

void RhythmDetector::RemoveDebugGraphs() {
    RELEASE(mGraphVelX);
    RELEASE(mGraphFolded);
    RELEASE(mGraphStdDev);
    RELEASE(mGraphStdDevBeat);
    RELEASE(mGraphRating);
}

void RhythmDetector::AddDebugGraph(
    float x, float y, float width, float height, Hmx::Color color
) {
    delete mGraphRating;
    mGraphRating = new DebugGraph(
        x,
        y,
        width,
        height,
        color,
        Hmx::Color(0.4f, 0.4f, 0.4f, 0.8f),
        120,
        0,
        2,
        MakeString(
            "beats %d  fold %d  dir %.1f %.1f %.1f",
            (int)mNumBeatsToCover,
            mBeatFold,
            mDir.x,
            mDir.y,
            mDir.z
        )
    );
    mGraphRating->SetUnk44(1);
}

void RhythmDetector::AddFullDebugGraphs() {
    if (gLog != -1) {
        Hmx::Color red(1, 0, 0, 1);
        delete mGraphVelX;
        mGraphVelX = new DebugGraph(
            0.1f, 0.0f, 0.8f, 0.06f, red, Hmx::Color(0, 0, 0, 0), 120, -1.1f, 1.1f, ""
        );
        mGraphVelX->SetUnk50(false);
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
    mThreadTotalSkelBeat = 0;
    mFrameData.clear();
    unk38.clear();
    mRecordData.mFrameData.clear();
    mRhythmRating = 0;
    mRhythmDecay = 0;
    mRecordData.mFinalized = true;
    mRecordData.mStartSeconds = -1;
    mRecordData.mEndSeconds = -1;
    mRecordData.mStartBeat = -1;
    mRecordData.mEndBeat = -1;
    mRecordData.mRating = -1;
    mRecordData.unk14 = -1;
    mFrames.clear();
    AddFullDebugGraphs();
}

void RhythmDetector::AddFrame(const BaseSkeleton &skel) {
    static UIPanel *rhythm_detector_panel =
        ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
    if (rhythm_detector_panel) {
        Vector3 v1a0[kNumJoints];
        for (int i = 0; i < kNumJoints; i++) {
            skel.JointPos(kCoordCamera, (SkeletonJoint)i, v1a0[i]);
        }
        float beat = TheTaskMgr.Beat();
        float secs = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
        float f13 = beat - unkaa8;
        if (f13 < 0) {
            f13 = 0;
            ClearData();
        }
        float f14 = 0;
        int i8 = -1;
        for (int i = 0; i < 8; i++) {
            if (0 <= unka84[i]) {
                float f10 = fabsf(secs - unka84[i] - 0.1f);
                if (f10 < f14 || i8 == -1) {
                    i8 = i;
                    f14 = f10;
                }
            }
        }
        if (i8 != -1) {
            auto harness_tmp_13 = Frame();
            mFrames.push_back(harness_tmp_13);
            if (mFrames.size() > 3) {
                mFrames.pop_front();
            }
            SetupFrame(
                mFrames.back(),
                mThreadTotalSkelBeat,
                f13,
                mCamJointPos[i8],
                v1a0,
                secs - unka84[i8]
            );
        }
        unkaa8 = beat;
        for (int i = 0; i < kNumJoints; i++) {
            mCamJointPos[unkaa4][i] = v1a0[i];
        }
        unka84[unkaa4] = secs;
        mThreadTotalSkelBeat += f13;
        unkaa4 = (unkaa4 + 1) % 8;
    }
}

const RhythmDetector::RecordData &RhythmDetector::GetRecord(
    float startBeat, float endBeat, bool finalize, Symbol s, TextStream *stream
) {
    if (mRecordData.mStartBeat == startBeat && mRecordData.mEndBeat == endBeat) {
        if (finalize) {
            AnalyzeData(
                unk38,
                mRecordData.mRating,
                mRecordData.unk14,
                mRhythmDecay,
                mToleranceFactor,
                true,
                s,
                false,
                mGraphVelX,
                gLog,
                stream
            );
            mRecordData.mFinalized = true;
        }
    } else {
        if (!mRecordData.mFinalized) {
            MILO_NOTIFY(
                "new rhythm detector window w/o finalization [%.1f,%.1f] to [%.1f, %.1f]",
                mRecordData.mStartBeat,
                mRecordData.mEndBeat,
                startBeat,
                endBeat
            );
        }
        ClearData();
        mRecordData.mStartBeat = startBeat;
        mRecordData.mEndBeat = endBeat;
        mRecordData.mFinalized = false;
        mRecordData.mEndSeconds = -1;
        mRecordData.mStartSeconds = -1;
        mRecordData.mFrameData.clear();
    }
    return mRecordData;
}

void RhythmDetector::ProcessFrames() {
    std::list<Frame> frames;
    frames.swap(mFrames);
    bool b2 = false;
    if (!frames.empty()) {
        float time = frames.front().mBeat;
        EraseNewerData(mRecordData.mFrameData, time);
        EraseNewerData(mFrameData, time);
        Frame &f13 = mLastFrame;
        FOREACH (it, frames) {
            Frame &cur = *it;
            mFrameData.push_back(cur);
            int i7 = f13.mBeat * 10;
            int i11 = ((int)(cur.mBeat * 10) - i7) % 40;
            if (!mLastFrame.mJointVelocities.empty()) {
                b2 = true;
                for (int i = 0; i < i11; i++) {
                    unk38.push_back(
                        BlendFrameDataToBeat(f13, cur, (float)(i + i7 + 1) / 10)
                    );
                }
            }
            f13 = cur;
            mLastFrame.mJointVelocities = cur.mJointVelocities;
        }
        if (frames.size() > 1) {
            frames.pop_back();
        }
        if (frames.size() > 1) {
            frames.pop_back();
        }
        mLastFrame = frames.back();
    }
    if (!mFrameData.empty()) {
        static UIPanel *rhythm_detector_panel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
        float f1;
        if (rhythm_detector_panel) {
            DataArray *typeDef = rhythm_detector_panel->TypeDef();
            static Symbol analyze_beat_frequency("analyze_beat_frequency");
            DataArray *freqArr = typeDef->FindArray(analyze_beat_frequency);
            static Symbol analyze_period_count("analyze_period_count");
            int i7 = typeDef->FindInt(analyze_period_count);
            freqArr->Int(freqArr->Size() - 1); // whoopsies
            int i11 = freqArr->Int(freqArr->Size() - 1);
            f1 = (float)i11 * (float)(i7 - 1) * 2;
        } else {
            f1 = 0;
        }
        auto found = mFrameData.end();
        FOREACH (it, mFrameData) {
            if (it->mBeat >= mFrameData.back().mBeat - f1) {
                found = it;
                break;
            }
        }
        mFrameData.erase(found, mFrameData.end());
        if (b2) {
            static Symbol blank("");
            AnalyzeData(
                unk38,
                mRecordData.mRating,
                mRecordData.unk14,
                mRhythmDecay,
                mToleranceFactor,
                false,
                blank,
                false,
                mGraphVelX,
                -1,
                nullptr
            );
        }
    }
}
