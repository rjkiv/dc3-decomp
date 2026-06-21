#include "flow/FlowTimer.h"
#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"

#pragma region EventTask

EventTask::EventTask(FlowTimer *t, ObjPtrVec<FlowNode> *v, TaskUnits u, float f4)
    : mOwner(this), unk40(v), mItr(0), unk48(f4) {
    mOwner = t;
    mItr = unk40->begin();
    TheTaskMgr.Start(this, u, 0);
}

void EventTask::Poll(float f1) {
    if (!mOwner) {
        MILO_NOTIFY("EventTask::Poll NULL mOwner");
    } else {
        for (mItr = unk40->begin(); mItr != unk40->end(); ++mItr) {
            mOwner->OnKeyframe(*mItr);
        }
        if (unk48 <= f1) {
            mOwner->OnTimerEnd();
        } else {
            return;
        }
    }
    delete this; // well then
}

#pragma endregion
#pragma region FlowTimer

FlowTimer::FlowTimer() : mStopMode(), mTask(this), mRate(), mTotalTime(0.0f) {}

FlowTimer::~FlowTimer() { TheFlowMgr->CancelCommand(this); }

BEGIN_PROPSYNCS(FlowTimer)
    SYNC_PROP(total_time, mTotalTime)
    SYNC_PROP(rate, (int &)mRate)
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowTimer)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mTotalTime;
    bs << mRate;
    bs << mStopMode;
END_SAVES

BEGIN_COPYS(FlowTimer)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowTimer, node)
    BEGIN_COPYING_MEMBERS_FROM(node)
        COPY_MEMBER_FROM(node, mTotalTime)
        COPY_MEMBER_FROM(node, mRate)
        COPY_MEMBER_FROM(node, mStopMode)
    END_COPYING_MEMBERS

END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(FlowTimer)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mTotalTime >> (int &)mRate;
    if (d.rev > 0)
        d >> (int &)mStopMode;
END_LOADS

bool FlowTimer::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    FlowNode::PushDrivenProperties();
    if (mTotalTime <= 0) {
        return false;
    } else {
        TheFlowMgr->QueueCommand(this, kQueue);
        return true;
    }
}

void FlowTimer::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    if (mTask) {
        delete mTask;
    }
    TheFlowMgr->CancelCommand(this);
    FlowNode::Deactivate(b);
}

void FlowTimer::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    mRunningNodes.remove(node);
    if (!mTask && mRunningNodes.empty()) {
        MILO_ASSERT_FMT(
            mFlowParent->HasRunningNode(this),
            "%s::HasRunningNode(%s)\n",
            PathName(mFlowParent),
            PathName(this)
        );
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowTimer::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (mStopMode == 0) {
        mRequestingStop = true;
        TheFlowMgr->QueueCommand(this, kIgnore);
        FlowNode::RequestStop();
    }
}

void FlowTimer::RequestStopCancel() {
    FLOW_LOG("RequestStopC\n");
    mRequestingStop = false;
    TheFlowMgr->QueueCommand(this, kQueue);
    FlowNode::RequestStopCancel();
}

void FlowTimer::Execute(FlowNode::QueueState state) {
    FLOW_LOG("Execute: state = %i\n", (int)state);
    if (IsRunning()) {
        if (state == kIgnore) {
            if (mTask) {
                delete mTask;
            }
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else {
        if (state == kQueue) {
            mTask = new EventTask(
                this, &mChildNodes, RndAnimatable::RateToTaskUnits(mRate), mTotalTime
            );
        } else if (state == kIgnore) {
            mFlowParent->ChildFinished(this);
        }
    }
}

bool FlowTimer::IsRunning() { return mTask || FlowNode::IsRunning(); }

void FlowTimer::OnKeyframe(FlowNode *node) {
    if (!node->IsRunning())
        FlowNode::ActivateChild(node);
}

void FlowTimer::OnTimerEnd() {
    if (mRunningNodes.empty()) {
        MILO_ASSERT(mFlowParent->HasRunningNode(this), 0x10d);
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

BEGIN_HANDLERS(FlowTimer)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

#pragma endregion
