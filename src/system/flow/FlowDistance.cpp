#include "flow/FlowDistance.h"
#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

FlowDistance::FlowDistance()
    : mOne(this, nullptr), mTwo(this, nullptr), mDistance(10), mPersistent(0),
      mIsRunning(0), mInRange(0), mRunInRange(1), mDriveIntensity(0), mIntensity(0) {}

FlowDistance::~FlowDistance() {}

BEGIN_HANDLERS(FlowDistance)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowDistance)
    SYNC_PROP(one, mOne)
    SYNC_PROP(two, mTwo)
    SYNC_PROP(distance, mDistance)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(run_in_range, mRunInRange)
    SYNC_PROP(drive_intensity, mDriveIntensity)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowDistance)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mOne;
    bs << mTwo;
    bs << mPersistent;
    bs << mDistance;
    bs << mRunInRange;
    bs << mDriveIntensity;
END_SAVES

BEGIN_COPYS(FlowDistance)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowDistance)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOne)
        COPY_MEMBER(mTwo)
        COPY_MEMBER(mPersistent)
        COPY_MEMBER(mDistance)
        COPY_MEMBER(mRunInRange)
        COPY_MEMBER(mDriveIntensity)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowDistance)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    mOne.LoadFromMainOrDir(d);
    mTwo.LoadFromMainOrDir(d);
    d >> mPersistent;
    d >> mDistance;
    d >> mRunInRange;
    d >> mDriveIntensity;
END_LOADS

bool FlowDistance::Activate() {
    FLOW_LOG("Activated\n");
    mRequestingStop = false;
    PushDrivenProperties();
    mRequestingStop = false;
    if (mOne && mTwo) {
        if (mPersistent) {
            TheFlowMgr->AddPollable(this);
            mIsRunning = true;
        }
        Vector3 diff;
        Subtract(mOne->WorldXfm().v, mTwo->WorldXfm().v, diff);
        Vector3 harness_reduction_vector;
        harness_reduction_vector.x = diff.y;
        harness_reduction_vector.y = diff.z;
        harness_reduction_vector.z = diff.x;
        mInRange = Length(harness_reduction_vector) > mDistance;
        Execute(kPolling);
        if (mPersistent) {
            return true;
        } else
            return FlowNode::IsRunning();
    } else {
        return false;
    }
}

void FlowDistance::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    TheFlowMgr->RemovePollable(this);
    mIsRunning = false;
    FlowNode::Deactivate(b);
}

void FlowDistance::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        if (mIsRunning) {
            TheFlowMgr->RemovePollable(this);
        }
        if (!mPersistent || mRequestingStop) {
            mIsRunning = false;
            mFlowParent->ChildFinished(this);
        }
    }
}

void FlowDistance::RequestStop() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStop();
}

void FlowDistance::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
}

bool FlowDistance::IsRunning() {
    if (mPersistent && mIsRunning)
        return true;
    return FlowNode::IsRunning();
}

void FlowDistance::UpdateIntensity() {
    float oldIntensity = FlowNode::sIntensity;
    FlowNode::sIntensity *= mIntensity;
    FlowNode::UpdateIntensity();
    FlowNode::sIntensity = oldIntensity;
}
