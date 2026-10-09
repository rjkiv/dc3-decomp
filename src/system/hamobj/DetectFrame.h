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

    const DancerFrame *mDancerFrame; // 0x0
    const MoveFrame *mMoveFrame; // 0x4
    float mSeconds; // 0x8
    MoveMirrored mMirrored; // 0xc
private:
    Vector3 mBestNodeErrors[kMaxNumErrorNodes]; // 0x10
    Vector3 mNodeComponentWeights[kMaxNumErrorNodes]; // 0x220
};

struct DetectFrameMoveIdxCmp {
    bool operator()(const DetectFrame &, int) const;
    bool operator()(int, const DetectFrame &) const;
};
