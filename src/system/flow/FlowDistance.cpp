#include "flow/FlowDistance.h"
#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

FlowDistance::FlowDistance()
    : mObj1(this, nullptr), mObj2(this, nullptr), mDistance(10), mPersistent(0), unka1(0),
      unka2(0), mRunInRange(1), mDriveIntensity(0), unka8(0) {}

FlowDistance::~FlowDistance() {}

BEGIN_HANDLERS(FlowDistance)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowDistance)
    SYNC_PROP(one, mObj1)
    SYNC_PROP(two, mObj2)
    SYNC_PROP(distance, mDistance)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(run_in_range, mRunInRange)
    SYNC_PROP(drive_intensity, mDriveIntensity)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowDistance)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mObj1;
    bs << mObj2;
    bs << mPersistent;
    bs << mDistance;
    bs << mRunInRange;
    bs << mDriveIntensity;
END_SAVES

BEGIN_COPYS(FlowDistance)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowDistance)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mObj1)
        COPY_MEMBER(mObj2)
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
    mObj1.LoadFromMainOrDir(bs);
    mObj2.LoadFromMainOrDir(bs);
    bs >> mDistance;
END_LOADS

bool FlowDistance::Activate() {
    FLOW_LOG("Activated\n");
    unk58 = false;
    PushDrivenProperties();
    unk58 = false;
    if (mObj1 && mObj2) {
        if (mPersistent) {
            TheFlowMgr->AddPollable(this);
            unka1 = true;
        }
        Vector3 diff;
        Subtract(mObj1->WorldXfm().v, mObj2->WorldXfm().v, diff);
        unka2 = Length(diff) > mDistance;
        Execute(kWhenAble);
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
    unka1 = false;
    FlowNode::Deactivate(b);
}

void FlowDistance::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        if (unka1) {
            TheFlowMgr->RemovePollable(this);
        }
        if (!mPersistent || unk58) {
            unka1 = false;
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
    if (mPersistent && unka1)
        return true;
    return FlowNode::IsRunning();
}

void FlowDistance::UpdateIntensity() {
    float oldIntensity = FlowNode::sIntensity;
    FlowNode::sIntensity *= unka8;
    FlowNode::UpdateIntensity();
    FlowNode::sIntensity = oldIntensity;
}
