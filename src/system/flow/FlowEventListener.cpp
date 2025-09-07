#include "flow/FlowEventListener.h"
#include "FlowEventListener.h"
#include "FlowTrigger.h"
#include "flow/FlowNode.h"
#include "flow/FlowQueueable.h"
#include "obj/Object.h"

FlowEventListener::FlowEventListener()
    : unkb4(0), mStartOnActivate(0), mEventCount(0), unkbc(0) {
    unkb1 = true;
}

FlowEventListener::~FlowEventListener() {}

BEGIN_HANDLERS(FlowEventListener)
    HANDLE_SUPERCLASS(FlowTrigger)
END_HANDLERS

BEGIN_PROPSYNCS(FlowEventListener)
    SYNC_PROP(event_count, mEventCount)
    SYNC_PROP(start_on_activate, mStartOnActivate)
    SYNC_SUPERCLASS(FlowTrigger)
END_PROPSYNCS

BEGIN_SAVES(FlowEventListener)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FlowTrigger)
    bs << mEventCount;
    bs << mStartOnActivate;
END_SAVES

BEGIN_COPYS(FlowEventListener)
    COPY_SUPERCLASS(FlowTrigger)
    CREATE_COPY(FlowEventListener)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mEventCount)
        COPY_MEMBER(mStartOnActivate)
    END_COPYING_MEMBERS
END_COPYS

bool FlowEventListener::Activate() {
    FLOW_LOG("Activate\n");
    unkb4 = true;
    unkbc = 0;
    RegisterEvents();
    if (mStartOnActivate) {
        ActivateTrigger();
    }
    return true;
}

void FlowEventListener::Deactivate(bool b1) {
    FLOW_LOG("Deactivated\n");
    FlowQueueable::Deactivate(b1);
    if (unkb4 && !b1) {
        unkb4 = false;
        UnregisterEvents();
    }
}

void FlowEventListener::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    if (unkb4) {
        FlowQueueable::ChildFinished(node);
    } else {
        FlowNode::ChildFinished(node);
    }
}

bool FlowEventListener::IsRunning() { return unkb4 || !mRunningNodes.empty(); }

void FlowEventListener::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (unkb4) {
        unkb4 = false;
        UnregisterEvents();
        if (mRunningNodes.empty()) {
            mParent->ChildFinished(this);
        } else {
            FlowQueueable::RequestStop();
        }
    }
}

void FlowEventListener::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    if (!unkb4) {
        unkb4 = true;
        FlowQueueable::RequestStopCancel();
        RegisterEvents();
    }
}
