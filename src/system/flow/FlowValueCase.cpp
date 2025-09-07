#include "flow/FlowValueCase.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowValueCase::FlowValueCase() : mValue(0) {}
FlowValueCase::~FlowValueCase() {}

BEGIN_HANDLERS(FlowValueCase)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowValueCase)
    SYNC_PROP(value, mValue)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowValueCase)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mValue;
END_SAVES

BEGIN_COPYS(FlowValueCase)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowValueCase)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mValue)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowValueCase)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (gRev < 1) {
        DataNode n;
        bs >> n;
        mValue = n.Float();
    } else
        bs >> mValue;
END_LOADS
