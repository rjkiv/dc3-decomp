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
        const FilterVersion *fv,
        float seconds,
        const MoveFrame *move_frame,
        const DancerFrame *dancer_frame,
        MoveMirrored mirror
    );
    void SetSecondsAndReset(float seconds);
    void Reset();
    bool HasScore() const;
    const Vector3 &BestNodeError(int node) const;
    const Vector3 &NodeComponentWeight(int node) const;
    void AddError(const Vector3 (&node_errors)[kMaxNumErrorNodes], float time_error);
    float LimbPSNR(const FilterVersion *filter_version, int limb_flags) const;
    float Score(const FilterVersion *filter_version, MoveMode mode) const;

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
