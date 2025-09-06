#include "flow/FlowSequence.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowSequence::FlowSequence()
    : unk5c(0), mLooping(0), mRepeats(0), unk68(0), mStopMode(kStopImmediate), unk70(0) {}

FlowSequence::~FlowSequence() {}

BEGIN_HANDLERS(FlowSequence)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSequence)
    SYNC_PROP(looping, mLooping)
    SYNC_PROP(repeats, mRepeats)
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSequence)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mLooping;
    bs << mRepeats;
    bs << mStopMode;
END_SAVES

BEGIN_COPYS(FlowSequence)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSequence)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLooping)
        COPY_MEMBER(mRepeats)
        COPY_MEMBER(mStopMode)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowSequence)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    bsrev >> mLooping;
    bs >> mRepeats;
    if (gRev > 0)
        bs >> (int &)mStopMode;
END_LOADS

void FlowSequence::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (mStopMode == kStopImmediate)
        FlowNode::RequestStop();
}

void FlowSequence::RequestStopCancel() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStopCancel();
}
