#include "hamobj/DetectFrame.h"
#include "ErrorNode.h"
#include "FilterVersion.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/HamMove.h"
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
    mMoveFrame = mf;
    mDancerFrame = df;
    mMirrored = mirror;
    const ErrorNode *const *nodes = fv->mNodes;
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
    MILO_ASSERT_RANGE(node, 0, kMaxNumErrorNodes, 0x72);
    return mBestNodeErrors[node];
}

const Vector3 &DetectFrame::NodeComponentWeight(int node) const {
    MILO_ASSERT_RANGE(node, 0, MoveFrame::kNumHam1Nodes, 0x80);
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

float DetectFrame::Score(const FilterVersion *fv, MoveMode mode) const {
    if (fv->mType == kFilterVersionHam1) {
        float f5 = 0;
        int numNodes = fv->NumNodes();
        for (int i = 0; i < numNodes; i++) {
            if (mMoveFrame->NodeWeightHam1(i, mode, mMirrored).mHasError) {
                f5 += mBestNodeErrors[i].x;
            }
        }
        return Max(0.0f, 1.0f - f5);
    } else {
        return LimbPSNR(fv, -1);
    }
}

float DetectFrame::LimbPSNR(const FilterVersion *filter_version, int i2) const {
    MILO_ASSERT(filter_version->mType == kFilterVersionHam2, 0x53);
    float f13 = 0;
    float f12 = 0;
    int numNodes = filter_version->NumNodes();
    unsigned int mfFlags = mMoveFrame->Flags();
    for (int i = 0; i < numNodes; i++) {
        ErrorNode *curErrorNode = filter_version->mNodes[i];
        if ((i2 == -1 || curErrorNode->GetFeedbackLimbs() & i2)
            && curErrorNode->Type() & mfFlags) {
            const Vector3 &nodeWeight = mMoveFrame->NodeWeight(i, mMirrored);
            float dot = Dot(nodeWeight, mBestNodeErrors[i]);
            f12 += dot * dot;
            f13 += Length(nodeWeight);
        }
    }
    if (f13 != 0) {
        float max_mse = 1;
        float mse = Min(max_mse, f12 / f13);
        MILO_ASSERT(mse >= 0, 0x20);
        MILO_ASSERT(mse <= max_mse, 0x21);
        if (mse == 0) {
            return 1000;
        } else {
            float mse_log = logf(max_mse / mse);
            return (mse_log * 10.0f) / logf(10.0f);
        }
    }
    return 0;
}

bool DetectFrameMoveIdxCmp::operator()(const DetectFrame &frame, int idx) const {
    return frame.mDancerFrame->mMoveIdx < idx;
}

bool DetectFrameMoveIdxCmp::operator()(int idx, const DetectFrame &frame) const {
    return idx < frame.mDancerFrame->mMoveIdx;
}
