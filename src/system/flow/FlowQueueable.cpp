#include "flow/FlowQueueable.h"
#include "flow/FlowNode.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include <list>

FlowQueueable::FlowQueueable() : mInterrupt(kImmediate) {}
FlowQueueable::~FlowQueueable() {}

BEGIN_HANDLERS(FlowQueueable)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowQueueable)
    SYNC_PROP(interrupt, (int &)mInterrupt)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowQueueable)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mInterrupt;
END_SAVES

BEGIN_COPYS(FlowQueueable)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowQueueable)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mInterrupt)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowQueueable)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> (int &)mInterrupt;
END_LOADS

void FlowQueueable::Deactivate(bool b1) {
    std::list<Hmx::Object *> objects(mQueuedListeners);
    mQueuedListeners.clear();
    while (objects.size() != 0) {
        ReleaseListener(objects.back());
        objects.pop_back();
    }
    FlowNode::Deactivate(b1);
}

void FlowQueueable::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    if (mInterrupt == 5) {
        FlowNode::ChildFinished(n);
    } else {
        mRunningNodes.remove(n);
        if (mRunningNodes.empty()) {
            if (mRequestingStop) {
                std::list<Hmx::Object *> objects(mQueuedListeners);
                mQueuedListeners.clear();
                while (objects.size() != 0) {
                    ReleaseListener(objects.back());
                    objects.pop_back();
                }
                if (mFlowParent && mRunningNodes.empty()
                    && mFlowParent->HasRunningNode(this)) {
                    FLOW_LOG("Releasing\n");
                    mFlowParent->ChildFinished(this);
                }
            } else if (mQueuedListeners.size() > 1) {
                Hmx::Object *obj = mQueuedListeners.front();
                bool found = false;
                for (auto it = ++mQueuedListeners.begin(); it != mQueuedListeners.end();
                     ++it) {
                    if (*it == obj) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ReleaseListener(mQueuedListeners.front());
                }
                mQueuedListeners.pop_front();
                ActivateTrigger();
            } else if (!mQueuedListeners.empty()) {
                ReleaseListener(mQueuedListeners.front());
                mQueuedListeners.pop_front();
            }
        }
    }
}

void FlowQueueable::RequestStop() { FlowNode::RequestStop(); }

void FlowQueueable::RequestStopCancel() {
    if (mRequestingStop) {
        FlowNode::RequestStopCancel();
    }
}

bool FlowQueueable::Activate(Hmx::Object *obj) {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (mRunningNodes.empty()) {
        if (obj) {
            mQueuedListeners.push_back(obj);
        } else {
            mQueuedListeners.push_back(nullptr);
        }
        if (ActivateTrigger()) {
            return true;
        } else {
            mQueuedListeners.clear();
            return false;
        }
    } else {
        switch (mInterrupt) {
        case kIgnore:
            FLOW_LOG("Ignoring trigger\n");
            ReleaseListener(obj);
            return false;
        case kQueue:
            FLOW_LOG("Queueing trigger\n");
            if (obj) {
                mQueuedListeners.push_back(obj);
            } else {
                mQueuedListeners.push_back(nullptr);
            }
            return true;
        case kQueueOne:
            FLOW_LOG("Queue One\n");
            while (mQueuedListeners.size() > 1) {
                ReleaseListener(mQueuedListeners.back());
                mQueuedListeners.pop_back();
            }
            if (obj) {
                mQueuedListeners.push_back(obj);
            } else {
                mQueuedListeners.push_back(nullptr);
            }
            return true;
        case kImmediate:
            FLOW_LOG("Immediate Interrupt\n");
            FlowQueueable::Deactivate(false);
            if (obj) {
                mQueuedListeners.push_back(obj);
            } else {
                mQueuedListeners.push_back(nullptr);
            }
            ActivateTrigger();
            return !mRunningNodes.empty();
        case kWhenAble:
            FLOW_LOG("When Able Interruption\n");
            while (mQueuedListeners.size() > 1) {
                ReleaseListener(mQueuedListeners.back());
                mQueuedListeners.pop_back();
            }
            if (obj) {
                mQueuedListeners.push_back(obj);
            } else {
                mQueuedListeners.push_back(nullptr);
            }
            RequestStop();
            return true;
        default:
            MILO_NOTIFY_ONCE("FlowQueueable: bad interupt value");
            return false;
        }
    }
}

void FlowQueueable::ReleaseListener(Hmx::Object *obj) {
    if (obj) {
        obj->Handle(Message("on_flow_finished", this), true);
    }
}
