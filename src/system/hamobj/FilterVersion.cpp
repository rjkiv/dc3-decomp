#include "hamobj/FilterVersion.h"
#include "ErrorNode.h"
#include "FilterVersion.h"
#include "obj/Data.h"
#include "os/Debug.h"

FilterVersion::FilterVersion(FilterVersionType t, const DataArray *cfg)
    : mFilterVersionSym(cfg->Sym(0)), mType(t) {
    static Symbol time_error("time_error");
    mScaleOp.Set(cfg->FindArray(time_error));
    static Symbol nodes("nodes");
    DataArray *error_nodes_data = cfg->FindArray(nodes);
    MILO_ASSERT(error_nodes_data->Size()-1 <= kMaxNumErrorNodes, 0x4E);
    int i;
    for (i = 0; i < error_nodes_data->Size() - 1; i++) {
        mErrorNodes[i] = ErrorNode::Create(error_nodes_data->Array(i + 1));
    }
    if (mType == kFilterVersionHam2) {
        sNumHam2Nodes = i;
    }
    for (; i < kMaxNumErrorNodes; i++) {
        mErrorNodes[i] = nullptr;
    }
}

FilterVersion::~FilterVersion() {
    for (int i = 0; i < kMaxNumErrorNodes; i++) {
        RELEASE(mErrorNodes[i]);
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
