#include "flow/FlowTimer.h"
#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"
#include "os/Debug.h"

FlowTimer::FlowTimer() : unk5c(0), unk60(this), mRate(0), mTotalTime(0.0f) {}

FlowTimer::~FlowTimer() { TheFlowMgr->CancelCommand(this); }

BEGIN_PROPSYNCS(FlowTimer)
    SYNC_PROP(total_time, mTotalTime)
    SYNC_PROP(rate, mRate)
    SYNC_PROP(stop_mode, unk5c)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowTimer)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mTotalTime;
    bs << mRate;
    bs << unk5c;
END_SAVES

BEGIN_COPYS(FlowTimer)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowTimer, node)
    BEGIN_COPYING_MEMBERS_FROM(node)
        COPY_MEMBER_FROM(node, mTotalTime)
        COPY_MEMBER_FROM(node, mRate)
        COPY_MEMBER_FROM(node, unk5c)
    END_COPYING_MEMBERS

END_COPYS

BEGIN_LOADS(FlowTimer)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    bs >> mTotalTime >> mRate;
    if (d.rev > 0)
        bs >> unk5c;
END_LOADS

bool FlowTimer::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    FlowNode::PushDrivenProperties();
    if (0.0f >= mTotalTime) {
        TheFlowMgr->QueueCommand(this, kQueue);
    }

    return true;
}

void FlowTimer::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    delete unk60;
    TheFlowMgr->CancelCommand(this);
    FlowNode::Deactivate(b);
}

void FlowTimer::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    mRunningNodes.remove(node);
}

void FlowTimer::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (unk5c == 0) {
        unk58 = true;
        TheFlowMgr->QueueCommand(this, kIgnore);
        FlowNode::RequestStop();
    }
}

void FlowTimer::RequestStopCancel() {
    FLOW_LOG("RequestStopC\n");
    unk58 = false;
    TheFlowMgr->QueueCommand(this, kQueue);
    FlowNode::RequestStopCancel();
}

void FlowTimer::Execute(FlowNode::QueueState state) {
    FLOW_LOG("Execute: state = %i\n", state);
    OnTimerEnd();
}

bool FlowTimer::IsRunning() { return unk60 || !mRunningNodes.empty(); }

void FlowTimer::OnKeyframe(FlowNode *node) {
    if (!node->HasRunningNode(node))
        FlowNode::ActivateChild(node);
}

void FlowTimer::OnTimerEnd() {
    if (mRunningNodes.empty()) {
        MILO_ASSERT(mFlowParent->HasRunningNode(this), 0x10d);

        FLOW_LOG("Timed Release From Parent \n");
    }
}

BEGIN_HANDLERS(FlowTimer)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS
