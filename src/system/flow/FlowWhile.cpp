#include "flow/FlowWhile.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/FlowSwitch.h"
#include "flow/FlowSwitchCase.h"
#include "flow/Flow.h"
#include "flow/PropertyEventListener.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"

FlowWhile::FlowWhile() : PropertyEventListener(this), mEntryCount(0) {}

FlowWhile::~FlowWhile() {}

BEGIN_HANDLERS(FlowWhile)
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowSwitch)
END_HANDLERS

BEGIN_PROPSYNCS(FlowWhile)
    SYNC_SUPERCLASS(FlowSwitch)
END_PROPSYNCS

BEGIN_SAVES(FlowWhile)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowSwitch)
END_SAVES

BEGIN_COPYS(FlowWhile)
    COPY_SUPERCLASS(FlowSwitch)
    CREATE_COPY(FlowWhile)
    BEGIN_COPYING_MEMBERS
        if (IsRunning()) {
            Deactivate(false);
        }
        UnregisterEvents(this);
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowWhile)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowSwitch)
    GenerateAutoNames(this, true);
END_LOADS

bool FlowWhile::Activate() {
    FLOW_LOG("Activated \n");
    mRequestingStop = false;
    if (IsRunning()) {
        MILO_NOTIFY(
            "FlowWhile re-entrance error, activated when already running, forcing stop, check your logic"
        );
        Deactivate(false);
        return false;
    } else if (mDrivenPropEntries.empty())
        return false;
    else {
        if (!mEventsRegistered) {
            RegisterEvents(this);
        }
        PushDrivenProperties();
        if (mValue.NotNull()) {
            if (unk64.Type() != mValue.Type()) {
                unk64 = mValue;
            }
        } else if (mValue.Type() == kDataObject) {
            unk64 = NULL_OBJ;
        } else {
            unk64 = 0;
        }
        DataNode n(unk64);
        unk64 = mValue;
        ActivateValueCases(mValue, n);
        if (mEventsRegistered) {
            return true;
        } else {
            return FlowNode::IsRunning();
        }
    }
}

void FlowWhile::Deactivate(bool b) {
    if (!b)
        PropertyEventListener::UnregisterEvents(this);
    mEventsRegistered = false;
    FlowNode::Deactivate(b);
}

void FlowWhile::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    if (!mEventsRegistered) {
        FlowNode::ChildFinished(n);
    } else {
        PushDrivenProperties();
        mRunningNodes.remove(n);
        FlowSwitchCase *fsc = static_cast<FlowSwitchCase *>(n);
        if (fsc && fsc->Op() == kTransition) {
            if (mValue != unk64) {
                DataNode dupe(unk64);
                unk64 = mValue;
                if (!ActivateTransitionCases(mValue, dupe)) {
                    ActivateValueCases(mValue, dupe);
                }
            } else {
                ActivateValueCases(mValue, mValue);
            }
        } else {
            if (mValue != unk64) {
                DataNode dupe(unk64);
                unk64 = mValue;
                if (!ActivateTransitionCases(mValue, dupe)) {
                    ActivateValueCases(mValue, dupe);
                }
            }
        }
    }
}

void FlowWhile::RequestStop() {
    UnregisterEvents(this);
    mEventsRegistered = false;
    if (!FlowNode::IsRunning()) {
        mFlowParent->ChildFinished(this);
    } else {
        FlowNode::RequestStop();
    }
}

void FlowWhile::RequestStopCancel() {
    FlowNode::RequestStopCancel();
    if (!mEventsRegistered)
        PropertyEventListener::RegisterEvents(this);
}

bool FlowWhile::IsRunning() {
    return (mEventsRegistered || FlowNode::IsRunning()) ? true : false;
}

void FlowWhile::MiloPreRun() {
    if (!IsRunning()) {
        UnregisterEvents(this);
        GenerateAutoNames(this, true);
    }
    FlowNode::MiloPreRun();
}

void FlowWhile::GenerateAutoNames(FlowNode *n, bool b) {
    if (GetDrivenEntry("value") && mChildNodes.size() != 0) {
        PropertyEventListener::GenerateAutoNames(this, true);
        FOREACH (it, mChildNodes) {
            PropertyEventListener::GenerateAutoNames(*it, false);
        }
    }
}

void FlowWhile::ReActivate() {
    FLOW_LOG("Reactivate\n");
    Timer timer;
    timer.Restart();
    PushDrivenProperties();
    mEntryCount++;
    if (mEntryCount > 8) {
        MILO_NOTIFY(
            "While reentrance count > 8 in flow %s, did you mean to use a switch? Aborting while node behavior",
            PathName(Dir())
        );
        mEntryCount--;
    } else {
        if (mRunningNodes.empty()) {
            if (unk64.Equal(mValue, nullptr, true)) {
                mEntryCount--;
                return;
            }
            if (!ActivateTransitionCases(mValue, unk64)) {
                ActivateValueCases(mValue, unk64);
            }
            unk64 = mValue;
        } else if (mFirstValidCaseOnly) {
            FlowSwitchCase *first = static_cast<FlowSwitchCase *>(mRunningNodes.front());
            if (first->Op() != kTransition) {
                FlowSwitchCase *cur = nullptr;
                FOREACH (it, mChildNodes) {
                    FlowSwitchCase *switchCase =
                        static_cast<FlowSwitchCase *>((FlowNode *)*it);
                    if (switchCase->IsValidCase(this, &mValue, &mValue, true)) {
                        cur = switchCase;
                        break;
                    }
                }
                if (cur != first) {
                    first->RequestStop();
                }
            }
        } else {
            FOREACH (it, mChildNodes) {
                FlowSwitchCase *cur = static_cast<FlowSwitchCase *>((FlowNode *)*it);
                if (!cur->IsValidCase(this, &mValue, &unk64, true)) {
                    if (cur->IsRunning()) {
                        cur->RequestStop();
                    }
                } else {
                    if (cur->IsRunning()) {
                        cur->RequestStopCancel();
                        continue;
                    } else {
                        ActivateChild(cur);
                        if (mRequestingStop)
                            break;
                    }
                }
            }
        }
        mEntryCount--;
        timer.Stop();
        FlowNode *n = this;
        Flow *topFlow;
        while (true) {
            topFlow = n->GetTopFlow();
            if (!topFlow->GetParent())
                break;
            n = topFlow->GetParent();
        }
        Symbol s = MakeString(
            "%s: %s->%s", ClassName(), topFlow->Dir()->Name(), topFlow->Name()
        );
        TheFlowMgr->AddEventTime(s, timer.Ms());
    }
}
