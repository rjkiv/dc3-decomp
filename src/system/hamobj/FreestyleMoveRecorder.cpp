#include "hamobj/FreestyleMoveRecorder.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"

FreestyleMoveRecorder *TheFreestyleMoveRecorder;

FreestyleMoveRecorder::FreestyleMoveRecorder()
    : unk4(0), unk8(0), unkc(0), unk18(0), unk20(-1), unk24(60), unk28(-1), unk2c(-1),
      unk30(15), unk34(-1), unk38(0), unk39(0), unk44(-1), unkb8(0) {
    unkbc = Hmx::Object::New<RndTex>();
    unkbc->SetBitmap(320, 240, 16, RndTex::kTexRegularLinear, false, nullptr);
}

void FreestyleMoveRecorder::Free() {
    unk28 = -1;
    unk2c = -1;

    for (int i = 4; i != 0; i--)
        unk48[unkb8].Free();
}

void FreestyleMoveRecorder::StartRecording() {
    unk34 = 0xffffffff;
    unk38 = false;
    unk28 = 0;
    unk2c = -1;

    if (unk20 == unkb8)
        return;

    unk48[unkb8].Init(unk24);
}

void FreestyleMoveRecorder::ClearRecording() {
    if (unk20 != unkb8)
        unk48[unkb8].Clear();

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

void FreestyleMoveRecorder::AssignStaticInstance() { TheFreestyleMoveRecorder = this; }
