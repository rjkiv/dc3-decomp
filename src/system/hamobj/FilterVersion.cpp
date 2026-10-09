#include "hamobj/FilterVersion.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/HamMove.h"
#include "obj/Data.h"
#include "os/Debug.h"

int FilterVersion::sNumHam2Nodes = 0;

FilterVersion::FilterVersion(FilterVersionType t, const DataArray *cfg)
    : mName(cfg->Sym(0)), mType(t) {
    static Symbol time_error("time_error");
    mTimeErrorOp.Set(cfg->FindArray(time_error));
    static Symbol nodes("nodes");
    DataArray *error_nodes_data = cfg->FindArray(nodes);
    MILO_ASSERT(error_nodes_data->Size()-1 <= kMaxNumErrorNodes, 0x4E);
    int i;
    for (i = 0; i < error_nodes_data->Size() - 1; i++) {
        mNodes[i] = ErrorNode::Create(error_nodes_data->Array(i + 1));
    }
    if (mType == kFilterVersionHam2) {
        sNumHam2Nodes = i;
    }
    for (; i < kMaxNumErrorNodes; i++) {
        mNodes[i] = nullptr;
    }
}

FilterVersion::~FilterVersion() {
    for (int i = 0; i < kMaxNumErrorNodes; i++) {
        RELEASE(mNodes[i]);
    }
}

FilterVersion *FilterVersion::Create(const DataArray *cfg) {
    FilterVersionType t = (FilterVersionType)cfg->FindInt("type");
    if (t == kFilterVersionHam1) {
        return new Ham1FilterVersion(t, cfg);
    } else if (t == kFilterVersionHam2) {
        return new Ham2FilterVersion(t, cfg);
    } else {
        MILO_FAIL("could not create filter version");
        return nullptr;
    }
}

void Ham1FilterVersion::NodeInput(
    int x, const DetectFrame *detectFrame, MoveMode mode, ErrorNodeInput &input
) const {
    const Ham1NodeWeight &ham1 =
        detectFrame->mMoveFrame->NodeWeightHam1(x, mode, detectFrame->mMirrored);
    input.Set(detectFrame->NodeComponentWeight(x), &ham1);
}

void Ham2FilterVersion::NodeInput(
    int x, const DetectFrame *detectFrame, MoveMode mode, ErrorNodeInput &input
) const {
    MoveMirrored mirror = detectFrame->mMirrored;
    input.Set(detectFrame->mMoveFrame->NodeInverseScale(x, mirror), nullptr);
}
