#include "flow/FlowOnStop.h"
#include "FlowOnStop.h"
#include "flow/FlowManager.h"
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

INIT_REVS(0, 0)

BEGIN_LOADS(FlowOnStop)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> (int &)mMode;
END_LOADS

bool FlowOnStop::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    unk60 = true;
    return true;
}

void FlowOnStop::Deactivate(bool b1) {
    FLOW_LOG("Deactivated, which can cause this node to activate!\n");
    if (unk60) {
        if (mMode != kRequestStopOnly) {
            FOREACH (it, mChildNodes) {
                ActivateChild(*it);
                if (unk58)
                    break;
            }
            FlowNode::Deactivate(b1);
        }
        unk60 = false;
    }
}

void FlowOnStop::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        unk60 = false;
        FLOW_LOG("Timed Release From Parent \n");
        Timer timer;
        timer.Reset();
        timer.Start();
        mFlowParent->ChildFinished(this);
        timer.Stop();
        TheFlowMgr->AddMs(timer.Ms());
    }
}

void FlowOnStop::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (mRunningNodes.empty()) {
        if (mMode != kDeactivateOnly) {
            unk58 = true;
            TheFlowMgr->QueueCommand(this, kQueue);
        } else {
            unk60 = false;
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
    if (qs == kQueue && unk58) {
        unk58 = false;
        unk60 = false;
        FlowNode::Activate();
        if (mRunningNodes.empty()) {
            FLOW_LOG("Timed Release From Parent \n");
            Timer timer;
            timer.Reset();
            timer.Start();
            mFlowParent->ChildFinished(this);
            timer.Stop();
            TheFlowMgr->AddMs(timer.Ms());
        }
    }
}

bool FlowOnStop::IsRunning() { return unk60 || FlowNode::IsRunning(); }
