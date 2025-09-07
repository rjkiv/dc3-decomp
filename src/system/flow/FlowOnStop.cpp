#include "flow/FlowOnStop.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowOnStop::FlowOnStop() : mMode(kAlways), unk60(0) {}
FlowOnStop::~FlowOnStop() {}

BEGIN_HANDLERS(FlowOnStop)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowOnStop)
    SYNC_PROP(mode, (int &)mMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowOnStop)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mMode;
END_SAVES

BEGIN_COPYS(FlowOnStop)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowOnStop)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMode)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowOnStop)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    bs >> (int &)mMode;
END_LOADS
