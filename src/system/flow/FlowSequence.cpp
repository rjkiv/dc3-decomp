#include "flow/FlowSequence.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "obj/Object.h"
#include "os/Debug.h"

FlowSequence::FlowSequence()
    : mItr(nullptr), mLooping(0), mRepeats(0), unk68(0), mStopMode(kStopImmediate),
      unk70(0) {}

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

INIT_REVS(1, 0)

BEGIN_LOADS(FlowSequence)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mLooping;
    d >> mRepeats;
    if (d.rev > 0)
        d >> (int &)mStopMode;
END_LOADS

bool FlowSequence::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (IsRunning()) {
        MILO_NOTIFY(
            "FlowSequence re-entrance error, activated when already running, deactivating and aborting, check your logic"
        );
        Deactivate(false);
    } else {
        if (unk68 == 0) {
            PushDrivenProperties();
        }
        unk68 = 0;
        if (!mChildNodes.empty()) {
            unk70 = true;
            for (mItr = mChildNodes.begin();; ++mItr) {
                do {
                    if (mItr == mChildNodes.end() || !mRunningNodes.empty()
                        || (ActivateChild(*mItr), mRequestingStop)) {
                        unk70 = false;
                        MILO_ASSERT(mRunningNodes.size() < 2, 0x50);
                        if (mItr == mChildNodes.end() && mRunningNodes.empty()) {
                            if (!mLooping && mRepeats == 0) {
                                return false;
                            }
                            MILO_NOTIFY_ONCE(
                                "Instant looping sequence in %s! Stopping Sequence",
                                GetOwnerFlow()->Name()
                            );
                        }
                        return !mRunningNodes.empty();
                    }
                } while (!mRunningNodes.empty());
            }
        }
    }
    return false;
}

void FlowSequence::ChildFinished(FlowNode *node) {
    FLOW_LOG(
        "Child Finished of class:%s ; potential advance of iterator\n", node->ClassName()
    );
    mRunningNodes.remove(node);
    MILO_ASSERT(mRunningNodes.empty(), 0x74);
    if (unk70)
        return;
    if (mRequestingStop) {
        mRequestingStop = false;
        FLOW_LOG("Releasing\n");
        mFlowParent->ChildFinished(this);
        return;
    }
    if (mItr != mChildNodes.end()) {
        ++mItr;
    }
    FLOW_LOG("Advancing sequence\n");
    unk70 = true;
    for (; mItr != mChildNodes.end(); ++mItr) {
        ActivateChild(*mItr);
        if (mRequestingStop || !mRunningNodes.empty())
            break;
    }
    unk70 = false;
    if (!mRequestingStop || !mRunningNodes.empty()) {
        if (mItr != mChildNodes.end())
            goto ret;
        if (!mLooping && unk68 >= mRepeats - 1) {
            MILO_ASSERT(mRunningNodes.empty(), 0xA1);
            FLOW_LOG("Releasing\n");
        } else if (Activate()) {
            unk68++;
            goto ret;
        }
    }
    mFlowParent->ChildFinished(this);
ret:
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
