#include "flow/FlowOnStop.h"
#include "FlowOnStop.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowOnStop::FlowOnStop() : mMode(kAlways), mIsRunning(0) {}
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

INIT_REVS(0, 0)

BEGIN_LOADS(FlowOnStop)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> (int &)mMode;
END_LOADS

bool FlowOnStop::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    mIsRunning = true;
    return true;
}

void FlowOnStop::Deactivate(bool b1) {
    FLOW_LOG("Deactivated, which can cause this node to activate!\n");
    if (mIsRunning) {
        if (mMode != kRequestStopOnly) {
            FOREACH (it, mChildNodes) {
                ActivateChild(*it);
                if (mRequestingStop)
                    break;
            }
            FlowNode::Deactivate(b1);
        }
        mIsRunning = false;
    }
}

void FlowOnStop::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        mIsRunning = false;
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowOnStop::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (mRunningNodes.empty()) {
        if (mMode != kDeactivateOnly) {
            mRequestingStop = true;
            TheFlowMgr->QueueCommand(this, kStarting);
        } else {
            mIsRunning = false;
            mFlowParent->ChildFinished(this);
        }
    } else {
        FlowNode::RequestStop();
    }
}

void FlowOnStop::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
}

void FlowOnStop::Execute(QueueState qs) {
    if (qs == kStarting && mRequestingStop) {
        mRequestingStop = false;
        mIsRunning = false;
        FlowNode::Activate();
        if (mRunningNodes.empty()) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    }
}

bool FlowOnStop::IsRunning() { return mIsRunning || FlowNode::IsRunning(); }
