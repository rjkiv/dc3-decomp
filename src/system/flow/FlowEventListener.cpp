#include "flow/FlowEventListener.h"
#include "FlowEventListener.h"
#include "FlowTrigger.h"
#include "flow/FlowManager.h"
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"
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

INIT_REVS(3, 0)

BEGIN_LOADS(FlowEventListener)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowTrigger)
    if (d.rev == 1) {
        bool b;
        d >> b;
        if (b) {
            mEventCount = 1;
        }
    } else {
        if (d.rev > 1) {
            d >> mEventCount;
        }
        if (d.rev > 2) {
            d >> mStartOnActivate;
        }
    }
    FOREACH (it, mTriggerEvents) {
        DataArray *def = GetEventEditorDef(*it);
        if (def) {
            for (int i = 2; i < def->Size(); i++) {
                DataArray *arr = def->Array(i);
                if (!Property(arr->Sym(0), false)) {
                    GetOwnerFlow()->SetProperty(arr->Sym(0), arr->Node(1));
                }
            }
        }
    }
END_LOADS

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
    if (!unkb4) {
        FlowNode::ChildFinished(node);
    } else {
        FlowQueueable::ChildFinished(node);
    }
}

void FlowEventListener::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (unkb4) {
        unkb4 = false;
        UnregisterEvents();
        if (mRunningNodes.empty()) {
            mFlowParent->ChildFinished(this);
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

bool FlowEventListener::IsRunning() { return unkb4 || !mRunningNodes.empty(); }

bool FlowEventListener::ActivateTrigger() {
    FLOW_LOG("Reactivate\n");
    mRequestingStop = false;
    unkbc++;
    if (mEventCount > 0 && unkbc >= mEventCount) {
        UnregisterEvents();
        unkb4 = false;
    }
    FlowNode::Activate();
    if (!FlowNode::IsRunning() && mEventCount > 0 && unkbc >= mEventCount) {
        FLOW_LOG("releasing\n");
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
    return FlowNode::IsRunning();
}
