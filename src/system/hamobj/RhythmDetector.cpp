#include "RhythmDetector.h"

#include "ui/UIPanel.h"
#include "obj/DataFunc.h"
#include "gesture/SkeletonUpdate.h"


void CameraToScreenUnit(Vector3 &vec, const Skeleton &skeleton, SkeletonJoint joint) {
    Vector2 skelPos;
    skeleton.ScreenPos(joint, skelPos);
    vec.y = -(skeleton.TrackedJoints()[joint].unk60.z) * 0.22977939;
    vec.x = (skelPos.x - 0.5f) * 2.0f;
    vec.z = (0.5f - skelPos.y) * 2.0f;
}

DebugGraph::DebugGraph(
    float f1,
    float f2,
    float f3,
    float f4,
    Hmx::Color color1,
    Hmx::Color color2,
    int i1,
    float f5,
    float f6,
    String s
)
    : mRect(f1, f2, f3, f4), mColorA(color1), mColorB(color2), unk38(i1), unk3c(f5), unk40(f6),
      unk44(3.4028235e+38), unk48(s), unk50(1) {}

DebugGraph::~DebugGraph() {}

RhythmDetector::RecordData::~RecordData() {}

namespace {
    int kAnalyzeJoints[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    int gDebugBone = -1;
    float gAdjust = 1;
    int gLog = -1;
    bool gClamp = true;
    const char **kConv;
    int kConvCount = 4;
    int kConvLen;

    void AnalyzeData(const std::vector<RhythmDetector::Frame> &frames, float &f1, float &f2, float &f3, float f4, bool b1, Symbol sym, bool b2, DebugGraph *dbg, int i1, TextStream *stream) {
        return;
    }

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
        //SomeGlobalOrSymbol = 1;
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

BEGIN_HANDLERS(RhythmDetector)
    HANDLE_ACTION(start_recording, StartRecording())
    HANDLE_ACTION(stop_recording, StopRecording())
    HANDLE_EXPR(is_recording, mRecording)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RhythmDetector)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_PROP(rhythm_rating, unk48);
    SYNC_PROP(rhythm_decay, unk4c)
    SYNC_PROP(num_beats_to_cover, unk44)
    SYNC_PROP(beat_fold, unk50)
    SYNC_PROP(tolerance_factor, unk54)
    SYNC_PROP(dir_x, unk58.x)
    SYNC_PROP(dir_y, unk58.y)
    SYNC_PROP(dir_z, unk58.z)
END_PROPSYNCS

BEGIN_COPYS(RhythmDetector)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RhythmDetector)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(unk44)
        COPY_MEMBER(unk50)
        COPY_MEMBER(unk54)
        COPY_MEMBER(unk58)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RhythmDetector)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(RndPollable)
    bs >> unk44;
    bs >> unk50;
    bs >> unk54;
    bs >> unk58.x;
    bs >> unk58.y;
    bs >> unk58.z;
    Normalize(unk58, unk58);
END_LOADS

BEGIN_SAVES(RhythmDetector)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(RndPollable)
    bs << unk44;
    bs << unk50;
    bs << unk54;
    bs << unk58.x;
    bs << unk58.y;
    bs << unk58.z;
END_SAVES

void EraseNewerData(std::vector<RhythmDetector::Frame> &vec, float time) {
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (it->unk0 >= time) {
            vec.erase(it, vec.end());
            break;
        }
    }
}

RhythmDetector::RhythmDetector() : unkc(false), mRecording(0), unk10(0), unk18(0), unk44(0), unk48(0), unk4c(0), unk50(2), unk54(0), unkaa8(0) {}

void RhythmDetector::Poll() {
    if (mRecording) {
        if (TheGestureMgr->GetSkeleton(unk10).IsTracked()) {
            if (mDebugGraphA) mDebugGraphA->Draw();
            if (mDebugGraphB) mDebugGraphB->Draw();
            if (mDebugGraphC) mDebugGraphC->Draw();
            if (mDebugGraphD) mDebugGraphD->Draw();
            if (mDebugGraphE) mDebugGraphE->Draw();

        }
    }
}

void RhythmDetector::Enter() {
    bool hasSkelInst = false;
    RndPollable::Enter();
    if (SkeletonUpdate::HasInstance()) {
        //somethign
        auto skel = SkeletonUpdate::InstanceHandle();
        hasSkelInst = true;
        bool hasSkelCallback = skel.HasCallback(this);
    }
    if (!SkeletonUpdate::HasInstance()) {
        auto skel = SkeletonUpdate::InstanceHandle();
        skel.AddCallback(this);
    }
}

float RhythmDetector::Groove() const {
    if (unkc) return unk48;
    else return 0;
}

float RhythmDetector::Freshness() const {
    if (unkc) return 1 - unk4c;
    else return 0;
}

Vector4 RhythmDetector::Data1(int i1) const {
    //Vector4 out;
    //out.x = unkaac[i1].x;
    //out.y = unkaac[i1].y;
    ////out.w = 0.0;
    //out.z = unkaac[i1].z;
    //return out;
    return Vector4(unkaac.x, unkaac.y, unkaac.z, 0.0);
 }

Vector4 RhythmDetector::Data2(int i1) const {
    return Vector4(0, 1, 1, 1);
}

void RhythmDetector::RemoveDebugGraphs() {
    delete mDebugGraphA;
    mDebugGraphA = nullptr;
    delete mDebugGraphB;
    mDebugGraphB = nullptr;
    delete mDebugGraphC;
    mDebugGraphC = nullptr;
    delete mDebugGraphD;
    mDebugGraphD = nullptr;
    delete mDebugGraphE;
    mDebugGraphE = nullptr;
}

void RhythmDetector::AddDebugGraph(float f1, float f2, float f3, float f4, Hmx::Color color) {
    if (mDebugGraphE) {
        delete mDebugGraphE;
    }
    String s = MakeString("beats %d fold %d dir %.1f %.1f %.1f", unk44, unk50, unk58.x, unk58.y, unk58.z);
    mDebugGraphE = new DebugGraph(f1, f2, f3, f4, Hmx::Color(0x3ecccccd, 0x3ecccccd, 0x3ecccccd), Hmx::Color(0x3ecccccd, 0x3ecccccd, 0x3ecccccd), s.length(), 0, 2, s);
    mDebugGraphE->SetUnk44(1.0);
}

void RhythmDetector::AddFullDebugGraphs() {
    if (gLog != -1) {
        if (mDebugGraphA) {
            delete mDebugGraphA;
        }
        auto mDebugGraphA = new DebugGraph(0.1,0.0,0.8,0.06, Hmx::Color(0.0,0.0,0.0,0.0), Hmx::Color(0.0,0.0,0.0,0.0), 0, -1.1, 1.1, "");
        mDebugGraphA->SetUnk50(false);
    }
}

void RhythmDetector::StartRecording() {
    char temp = mRecording;
    mRecording = temp + 1;
    if (mRecording == 1) {
        AddFullDebugGraphs();
        unkaa8 = TheTaskMgr.Beat();
        ClearData();
    }
    MILO_ASSERT(mRecording >= 1, 0x3cc);
    MILO_ASSERT(mRecording <= 2, 0x3cd);
}

void RhythmDetector::StopRecording() {
    char temp = mRecording;
    mRecording = temp - 1;
    if (mRecording == 0) {
        unkaa8 = TheTaskMgr.Beat();
        ClearData();
    }
    MILO_ASSERT(mRecording >= 0, 0x3da);
    MILO_ASSERT(mRecording <= 1, 0x3db);
}

void RhythmDetector::ClearData() {
    float temp = 0.0;
    unk68 = 0.0;
    unk2c.clear();
    unk38.clear();
    unkc08.clear();
    unk48 = 0.0;
    unk4c = 0.0;
    unkc04 = true;
    mRecordData.unkbf4 = -1.0;
    mRecordData.unkbf8 = -1.0;
    mRecordData.unkbec = -1.0;
    mRecordData.unkbf0 = -1.0;
    mRecordData.unkbfc = -1.0;
    mRecordData.unkc00 = -1.0;
    unk14.clear();
    AddFullDebugGraphs();
}

void RhythmDetector::AddFrame(BaseSkeleton const &skel) {
    auto panel = ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
    if (panel) {
        Vector3 jointPosVec;
        for (int skelJoint = 0; skelJoint < kNumJoints; skelJoint++) {
            skel.JointPos(kCoordCamera, static_cast<SkeletonJoint>(skelJoint), jointPosVec);
        }
        float beat = TheTaskMgr.Beat();
        float seconds =TheTaskMgr.Seconds(TaskMgr::kRealTime);
        float beatDiff = beat - unkaa8;
        if (beatDiff < 0.0) {
            ClearData();
            beatDiff = 0.0;
        }
        int temp = -1;
        int anotherInt = 0;
        float something = 0.0;
        for (int i = 0; i < 8; i++) {
            if (0.0 < unka80.back() && ((fabs(seconds - unka80.back()) - 0.1) < something || temp == -1)) {
                something = fabs(seconds - unka80.back()) - 0.1;
                temp = anotherInt;
                anotherInt++;
            }
        }
        if (temp != -1) {

        }
    }

}

const RhythmDetector::RecordData &
RhythmDetector::GetRecord(float f1, float f2, bool b, Symbol sym, TextStream *stream) {
    RecordData ret = mRecordData;
    if (mRecordData.unkbec == f1 && mRecordData.unkbf8 == f2) {
        if (stream) {
            AnalyzeData(unk38, mRecordData.unkbfc, mRecordData.unkc00, unk4c, unk54, mDebugGraphA->GetUnk50(), 0, b, 0, mDebugGraphA->GetUnk38(), stream);
            unkc04 = true;
        }
    }
    else {
        if (unkc04 == false) {
            MILO_NOTIFY("new rhythm detector window w/o finalization [%.1f,%.1f] to [%.1f, %.1f]", mRecordData.unkbec, mRecordData.unkbf0, f1, f2);
        }
        ClearData();
        mRecordData.unkbf0 = f2;
        unkc04 = false;
        mRecordData.unkbf4 = -1.0;
        mRecordData.unkbf8 = -1.0;
        unkc08.clear();
    }
    return ret;
}