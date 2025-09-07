#include "flow/FlowSequence.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"
#include "os/Debug.h"

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

void FlowSequence::ChildFinished(FlowNode *node) {
    FLOW_LOG(
        "Child Finished of class:%s ; potential advance of iterator\n", node->ClassName()
    );
    mRunningNodes.remove(node);
    MILO_ASSERT(mRunningNodes.empty(), 0x74);
    if (unk70)
        return;
    if (unk58) {
        unk58 = false;
        MILO_LOG("Releasing\n");
        mParent->ChildFinished(this);
        return;
    }
    unk5c = (unk5c + 1) % 0x14;
    FLOW_LOG("Advancing sequence\n");
    unk70 = true;
    // more...
    MILO_ASSERT(mRunningNodes.size() < 2, 0xA6);
}

void FlowSequence::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (mStopMode == kStopImmediate)
        FlowNode::RequestStop();
}

void FlowSequence::RequestStopCancel() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStopCancel();
}
