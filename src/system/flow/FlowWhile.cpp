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

FlowWhile::FlowWhile() : PropertyEventListener(this), mReentranceCount(0) {}

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
        if (!mRegistered) {
            RegisterEvents(this);
        }
        PushDrivenProperties();
        if (mCurrentValue.NotNull()) {
            if (mLastValue.Type() != mCurrentValue.Type()) {
                mLastValue = mCurrentValue;
            }
        } else if (mCurrentValue.Type() == kDataObject) {
            mLastValue = NULL_OBJ;
        } else {
            mLastValue = 0;
        }
        DataNode n(mLastValue);
        mLastValue = mCurrentValue;
        ActivateValueCases(mCurrentValue, n);
        if (mRegistered) {
            return true;
        } else {
            return FlowNode::IsRunning();
        }
    }
}

void FlowWhile::Deactivate(bool b) {
    if (!b)
        PropertyEventListener::UnregisterEvents(this);
    mRegistered = false;
    FlowNode::Deactivate(b);
}

void FlowWhile::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    if (!mRegistered) {
        FlowNode::ChildFinished(n);
    } else {
        PushDrivenProperties();
        mRunningNodes.remove(n);
        FlowSwitchCase *fsc = static_cast<FlowSwitchCase *>(n);
        if (fsc && fsc->GetOperator() == kTransition) {
            if (mCurrentValue != mLastValue) {
                DataNode dupe(mLastValue);
                mLastValue = mCurrentValue;
                if (!ActivateTransitionCases(mCurrentValue, dupe)) {
                    ActivateValueCases(mCurrentValue, dupe);
                }
            } else {
                ActivateValueCases(mCurrentValue, mCurrentValue);
            }
        } else {
            if (mCurrentValue != mLastValue) {
                DataNode dupe(mLastValue);
                mLastValue = mCurrentValue;
                if (!ActivateTransitionCases(mCurrentValue, dupe)) {
                    ActivateValueCases(mCurrentValue, dupe);
                }
            }
        }
    }
}

void FlowWhile::RequestStop() {
    UnregisterEvents(this);
    mRegistered = false;
    if (!FlowNode::IsRunning()) {
        mFlowParent->ChildFinished(this);
    } else {
        FlowNode::RequestStop();
    }
}

void FlowWhile::RequestStopCancel() {
    FlowNode::RequestStopCancel();
    if (!mRegistered)
        PropertyEventListener::RegisterEvents(this);
}

bool FlowWhile::IsRunning() {
    return (mRegistered || FlowNode::IsRunning()) ? true : false;
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
    mReentranceCount++;
    if (mReentranceCount > 8) {
        MILO_NOTIFY(
            "While reentrance count > 8 in flow %s, did you mean to use a switch? Aborting while node behavior",
            PathName(Dir())
        );
        mReentranceCount--;
        return;
    }
    if (mRunningNodes.empty()) {
        if (mLastValue.Equal(mCurrentValue, nullptr, true)) {
            mReentranceCount--;
            return;
        }
        if (!ActivateTransitionCases(mCurrentValue, mLastValue)) {
            ActivateValueCases(mCurrentValue, mLastValue);
        }
        mLastValue = mCurrentValue;
    } else if (mFirstValidCaseOnly) {
        FlowSwitchCase *first = static_cast<FlowSwitchCase *>(mRunningNodes.front());
        if (first->GetOperator() != kTransition) {
            FlowSwitchCase *cur = nullptr;
            FOREACH (it, mChildNodes) {
                FlowSwitchCase *switchCase =
                    static_cast<FlowSwitchCase *>((FlowNode *)*it);
                if (switchCase->IsValidCase(this, &mCurrentValue, &mCurrentValue, true)) {
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
            if (!cur->IsValidCase(this, &mCurrentValue, &mLastValue, true)) {
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
    mReentranceCount--;
    timer.Stop();
    FlowNode *n = this;
    Flow *topFlow;
    while (true) {
        topFlow = n->GetTopFlow();
        if (!topFlow->GetParent())
            break;
        n = topFlow->GetParent();
    }
    Symbol s =
        MakeString("%s: %s->%s", ClassName(), topFlow->Dir()->Name(), topFlow->Name());
    TheFlowMgr->AddEventTime(s, timer.Ms());
}
