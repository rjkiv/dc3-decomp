#pragma once
#include "FreestyleMove.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "rndobj/Tex.h"
#include "utl/Str.h"

// size 0x110
class FreestyleMoveRecorder : public SkeletonCallback {
public:
    struct JointAngle {
        int unk0;
    };
    struct JointPos {
        int unk0;
        int unk4;
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
    void StartPlayback(bool);
    void StopPlayback();
    void ClearDancerTake();
    BaseSkeleton *GetLiveSkeleton();
    void AssignStaticInstance();

    void SetVal44(int i) { unk44 = i; } // change once context found

private:
    float unk4;
    int unk8;
    int unkc;
    String unk10;
    int unk18;
    int unk1c;
    int unk20;
    int unk24;
    float unk28;
    float unk2c;
    float unk30;
    int unk34;
    bool unk38;
    bool unk39;
    Symbol unk3c;
    int unk40;
    int unk44;
    FreestyleMove unk48[4]; // 0x48
    int unkb8;
    RndTex *unkbc;
    FreestyleMoveFrame *unkc0;
    int unkc4;
    int unkc8;
    std::vector<JointAngle> unkcc;
    std::vector<SkeletonJoint> unkd8;
    FreeStyleFrameScores unke4[2];
    std::vector<JointPos> unk104;
};

extern FreestyleMoveRecorder *TheFreestyleMoveRecorder;
