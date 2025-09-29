#include "hamobj/DetectFrame.h"
#include "ErrorNode.h"
#include "FilterVersion.h"
#include "hamobj/ErrorNode.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "os/Debug.h"

DetectFrame::DetectFrame() { Reset(); }

void DetectFrame::Reset() {
    for (int i = 0; i < kMaxNumErrorNodes; i++) {
        mBestNodeErrors[i].Set(kHugeFloat, kHugeFloat, kHugeFloat);
    }
}

void DetectFrame::Reset(
    const FilterVersion *fv,
    float secs,
    const MoveFrame *mf,
    const DancerFrame *df,
    MoveMirrored mirror
) {
    Reset();
    mSeconds = secs;
    unk4 = mf;
    unk0 = df;
    unkc = mirror;
    const ErrorNode *const *nodes = fv->mErrorNodes;
    if (fv->mType == kFilterVersionHam1) {
        for (int i = 0; i < MoveFrame::kNumHam1Nodes; i++) {
            mNodeComponentWeights[i].y = 1;
            Vector3 v;
            if (nodes[i]->XZErrorAxis(v, df->mSkeleton)) {
                XZErrorWeight(v, mNodeComponentWeights[i].x, mNodeComponentWeights[i].z);
            } else {
                mNodeComponentWeights[i].x = mNodeComponentWeights[i].z = 1;
            }
        }
    }
}

void DetectFrame::SetSecondsAndReset(float secs) {
    mSeconds = secs;
    Reset();
}

bool DetectFrame::HasScore() const { return mBestNodeErrors[0].x != kHugeFloat; }

const Vector3 &DetectFrame::BestNodeError(int node) const {
    MILO_ASSERT((0) <= (node) && (node) < (kMaxNumErrorNodes), 0x72);
    return mBestNodeErrors[node];
}

const Vector3 &DetectFrame::NodeComponentWeight(int node) const {
    MILO_ASSERT((0) <= (node) && (node) < (MoveFrame::kNumHam1Nodes), 0x80);
    return mNodeComponentWeights[node];
}

void DetectFrame::AddError(const Vector3 (&errors)[kMaxNumErrorNodes], float f) {
    for (int i = 0; i < kMaxNumErrorNodes; i++) {
        for (int j = 0; j < 3; j++) {
            float sum = errors[i][j] + f;
            if (sum < mBestNodeErrors[i][j]) {
                mBestNodeErrors[i][j] = sum;
            }
        }
    }
}

bool DetectFrameMoveIdxCmp::operator()(const DetectFrame &frame, int idx) const {
    return frame.GetDancerFrame()->unk0 < idx;
}

bool DetectFrameMoveIdxCmp::operator()(int idx, const DetectFrame &frame) const {
    return idx < frame.GetDancerFrame()->unk0;
}
