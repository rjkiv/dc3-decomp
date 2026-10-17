#include "flow/FlowSequence.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "obj/Object.h"
#include "os/Debug.h"

FlowSequence::FlowSequence()
    : mSeqItr(nullptr), mLooping(0), mRepeatCount(0), mCurCycle(0),
      mStopMode(kStopImmediate), mInActivation(0) {}

FlowSequence::~FlowSequence() {}

BEGIN_HANDLERS(FlowSequence)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSequence)
    SYNC_PROP(looping, mLooping)
    SYNC_PROP(repeats, mRepeatCount)
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSequence)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mLooping;
    bs << mRepeatCount;
    bs << mStopMode;
END_SAVES

BEGIN_COPYS(FlowSequence)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSequence)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLooping)
        COPY_MEMBER(mRepeatCount)
        COPY_MEMBER(mStopMode)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(FlowSequence)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mLooping;
    d >> mRepeatCount;
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
        return false;
    }
    if (mCurCycle == 0) {
        PushDrivenProperties();
    }
    mCurCycle = 0;
    if (mChildNodes.empty()) {
        return false;
    }
    mInActivation = true;
    for (mSeqItr = mChildNodes.begin();; ++mSeqItr) {
        do {
            if (mSeqItr == mChildNodes.end() || !mRunningNodes.empty()
                || (ActivateChild(*mSeqItr), mRequestingStop)) {
                mInActivation = false;
                MILO_ASSERT(mRunningNodes.size() < 2, 0x50);
                if (mSeqItr == mChildNodes.end() && mRunningNodes.empty()) {
                    if (!mLooping && mRepeatCount == 0) {
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

void FlowSequence::ChildFinished(FlowNode *node) {
    FLOW_LOG(
        "Child Finished of class:%s ; potential advance of iterator\n", node->ClassName()
    );
    mRunningNodes.remove(node);
    MILO_ASSERT(mRunningNodes.empty(), 0x74);
    if (mInActivation)
        return;
    if (mRequestingStop) {
        mRequestingStop = false;
        FLOW_LOG("Releasing\n");
        mFlowParent->ChildFinished(this);
        return;
    }
    if (mSeqItr != mChildNodes.end()) {
        ++mSeqItr;
    }
    FLOW_LOG("Advancing sequence\n");
    mInActivation = true;
    for (; mSeqItr != mChildNodes.end(); ++mSeqItr) {
        ActivateChild(*mSeqItr);
        if (mRequestingStop || !mRunningNodes.empty())
            break;
    }
    mInActivation = false;
    if (!mRequestingStop || !mRunningNodes.empty()) {
        if (mSeqItr != mChildNodes.end())
            goto ret;
        if (!mLooping && mCurCycle >= mRepeatCount - 1) {
            MILO_ASSERT(mRunningNodes.empty(), 0xA1);
            FLOW_LOG("Releasing\n");
        } else {
            int count = mCurCycle;
            if (Activate()) {
                mCurCycle = count + 1;
                goto ret;
            }
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
