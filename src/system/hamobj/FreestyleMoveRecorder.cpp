#include "hamobj/FreestyleMoveRecorder.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/FreestyleMove.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "rndobj/Tex.h"
#include "utl/FileStream.h"
#include "utl/Symbol.h"

DancerSkeleton sLastComparedDancerSkel;

FreestyleMoveRecorder::FreestyleMoveRecorder()
    : unk4(0), unk8(0), unkc(0), unk18(0), unk20(-1), unk24(60), unk28(-1), unk2c(-1),
      unk30(15), unk34(-1), unk38(0), unk39(0), unk44(-1), unkb8(0) {
    unkbc = Hmx::Object::New<RndTex>();
    unkbc->SetBitmap(320, 240, 16, RndTex::kRegularLinear, false, nullptr);

    JointAngle angle;
    angle.mJoint = kJointHandRight;
    unkcc.push_back(angle);
    angle.mJoint = kJointHandLeft;
    unkcc.push_back(angle);
    angle.mJoint = kJointAnkleRight;
    unkcc.push_back(angle);
    angle.mJoint = kJointAnkleLeft;
    unkcc.push_back(angle);
    angle.mJoint = kJointKneeRight;
    unkcc.push_back(angle);
    angle.mJoint = kJointKneeLeft;
    unkcc.push_back(angle);
    unkd8.push_back(kJointHandRight); // 11
    unkd8.push_back(kJointHandLeft); // 7
    unkd8.push_back(kJointAnkleRight); // 17
    unkd8.push_back(kJointAnkleLeft); // 14
    unkd8.push_back(kJointHead); // 3
    unkd8.push_back(kJointHipCenter); // 0
    JointPos pos;
    pos.unk0 = 11;
    pos.unk4 = 2;
    unk104.push_back(pos);
    pos.unk0 = 7;
    pos.unk4 = 1;
    unk104.push_back(pos);
    pos.unk0 = 9;
    pos.unk4 = 2;
    unk104.push_back(pos);
    pos.unk0 = 5;
    pos.unk4 = 1;
    unk104.push_back(pos);
    pos.unk0 = 17;
    pos.unk4 = 4;
    unk104.push_back(pos);
    pos.unk0 = 14;
    pos.unk4 = 3;
    unk104.push_back(pos);
    pos.unk0 = 16;
    pos.unk4 = 4;
    unk104.push_back(pos);
    pos.unk0 = 13;
    pos.unk4 = 3;
    unk104.push_back(pos);
    unkc0 = new FreestyleMoveFrame[unk24];
    DataRegisterFunc("bam_record_attempt", OnRecordAttempt);
    DataRegisterFunc("bam_write_created", OnWriteCreated);
    DataRegisterFunc("bam_read_created", OnReadCreated);
    DataRegisterFunc("bam_read_attempt", OnReadAttempt);
    DataRegisterFunc("bam_clear", OnClearAttempt);
}

FreestyleMoveRecorder::~FreestyleMoveRecorder() {
    delete unkbc;
    delete[] unkc0;
    delete[] unk18;
    delete[] unk8;
}

void FreestyleMoveRecorder::Free() {
    unk28 = -1;
    unk2c = -1;
    for (int i = 4; i != 0; i--) {
        unk48[unkb8].Free();
    }
}

void FreestyleMoveRecorder::StartRecording() {
    unk34 = 0xffffffff;
    unk38 = false;
    unk28 = 0;
    unk2c = -1;
    if (unk20 != unkb8) {
        unk48[unkb8].Init(unk24);
    }
}

void FreestyleMoveRecorder::ClearRecording() {
    if (unk20 != unkb8) {
        unk48[unkb8].Clear();
    }
    unkc8 = 0;
}

void FreestyleMoveRecorder::StartRecordingDancerTake() {
    StartRecording();
    unk38 = true;
}

void FreestyleMoveRecorder::StartPlayback(bool param_1) {
    unk39 = param_1;
    unk2c = 0;
}

void FreestyleMoveRecorder::StopPlayback() { unk2c = -1; }

void FreestyleMoveRecorder::ClearDancerTake() { unkc4 = 0; }

void FreestyleMoveRecorder::AssignStaticInstance() { sInstance = this; }

void FreestyleMoveRecorder::UpdateRecordingAttempt(
    const BaseSkeleton *skeleton, float f2
) {
    if (unk10 != gNullStr) {
        unk18[unk1c].skeleton.Set(*skeleton);
        unk18[unk1c].unk2d8 = f2;
        unk1c++;
    }
}

void FreestyleMoveRecorder::RecordMoveAttempt(String str) {
    unk10 = str;
    delete[] unk18;
    unk18 = new FreestyleMoveFrame[480];
    unk1c = 0;
}

void FreestyleMoveRecorder::WriteRecordedMoveAttempt() {
    WriteFreestyleMoveClip(unk10, unk1c, unk18);
    unk10 = gNullStr;
    delete[] unk18;
    unk18 = nullptr;
    unk1c = 0;
}

void FreestyleMoveRecorder::ClearFreestyleMoveClip() {
    delete[] unk8;
    unk8 = nullptr;
    unkc = 0;
}

void FreestyleMoveRecorder::PlaybackComplete() {
    if (unk10 != gNullStr) {
        WriteRecordedMoveAttempt();
    }
}

void FreestyleMoveRecorder::ClearFrameScores() {
    for (int i = 0; i < 2; i++) {
        unke4[i].Clear();
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
    stream << unk3c;
    stream << framecount;
    for (int i = 0; i < framecount; i++) {
        frames[i].skeleton.Write(stream);
        stream << frames[i].unk2d8;
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
        frames[i].skeleton.Read(stream);
        stream >> frames[i].unk2d8;
    }
    MILO_LOG("Loaded clip that was recorded with %s, framecount: %d\n", s, framecount);
}

DataNode FreestyleMoveRecorder::OnRecordAttempt(DataArray *a) {
    String str;
    if (a->Size() >= 2) {
        str = a->Str(1);
    } else {
        str = sInstance->unk3c.Str();
        str += "_attempt_";
        DateTime dt;
        GetDateAndTime(dt);
        str += MakeString("%02d%02d_%02d%02d", dt.Month(), dt.mDay, dt.mHour, dt.mMin);
    }
    sInstance->RecordMoveAttempt(str);
    return 0;
}

DataNode FreestyleMoveRecorder::OnWriteCreated(DataArray *a) {
    String str;
    if (a->Size() >= 2) {
        str = a->Str(1);
    } else {
        str = sInstance->unk3c.Str();
        str += "_created_";
        DateTime dt;
        GetDateAndTime(dt);
        str += MakeString("%02d%02d_%02d%02d", dt.Month(), dt.mDay, dt.mHour, dt.mMin);
    }
    sInstance->WriteFreestyleMoveClip(
        str,
        sInstance->unk48[sInstance->unkb8].mNumFrames,
        sInstance->unk48[sInstance->unkb8].unk18
    );
    return 0;
}

DataNode FreestyleMoveRecorder::OnReadCreated(DataArray *a) {
    int framecount;
    sInstance->ReadFreestyleMoveClip(
        a->Str(1), framecount, sInstance->unk48[sInstance->unkb8].unk18
    );
    sInstance->unk48[sInstance->unkb8].Init(sInstance->unk24);
    sInstance->unk48[sInstance->unkb8].mNumFrames = framecount;
    sInstance->unk20 = sInstance->unkb8;
    return 0;
}

DataNode FreestyleMoveRecorder::OnReadAttempt(DataArray *a) {
    delete[] sInstance->unk8;
    sInstance->unk8 = new FreestyleMoveFrame[480];
    sInstance->ReadFreestyleMoveClip(a->Str(1), sInstance->unkc, sInstance->unk8);
    return 0;
}

DataNode FreestyleMoveRecorder::OnClearAttempt(DataArray *a) {
    sInstance->ClearFreestyleMoveClip();
    return 0;
}
