#pragma once
#include "ErrorNode.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include "math/Vec.h"

// size 0x430
class DetectFrame {
public:
    DetectFrame();
    void Reset(
        const FilterVersion *, float, const MoveFrame *, const DancerFrame *, MoveMirrored
    );
    void SetSecondsAndReset(float);
    void Reset();
    bool HasScore() const;
    const Vector3 &BestNodeError(int) const;
    const Vector3 &NodeComponentWeight(int) const;
    void AddError(const Vector3 (&)[kMaxNumErrorNodes], float);
    float LimbPSNR(const FilterVersion *, int) const;
    float Score(const FilterVersion *, MoveMode) const;
    const DancerFrame *GetDancerFrame() const { return unk0; }
    const MoveFrame *GetMoveFrame() const { return unk4; }
    MoveMirrored Mirror() const { return unkc; }

protected:
    const DancerFrame *unk0; // 0x0
    const MoveFrame *unk4; // 0x4
    float mSeconds; // 0x8
    MoveMirrored unkc; // 0xc
    Vector3 mBestNodeErrors[kMaxNumErrorNodes]; // 0x10, 33*16=0x210, ends at 0x220
    Vector3 mNodeComponentWeights[MoveFrame::kNumHam1Nodes]; // 0x220, 16*16=0x100, ends at 0x320
    char unk320[0x110]; // 0x320, unknown data to reach size 0x430
};

struct DetectFrameMoveIdxCmp {
    bool operator()(const DetectFrame &, int) const;
    bool operator()(int, const DetectFrame &) const;
};
