#include "flow/FlowSlider.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/PropertyEventListener.h"
#include "flow/Flow.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "os/Debug.h"

FlowSlider::FlowSlider()
    : PropertyEventListener(this), mPersistent(1), mAlwaysRun(0), mValue(0),
      mEaseType(kEasePolyOut), mEasePower(2) {
    UpdateEase();
}

FlowSlider::~FlowSlider() {}

BEGIN_HANDLERS(FlowSlider)
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSlider)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(always_run, mAlwaysRun)
    SYNC_PROP_MODIFY(ease, (int &)mEaseType, UpdateEase())
    SYNC_PROP(ease_power, mEasePower)
    SYNC_PROP_MODIFY(value, mValue, UpdateActivations())
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSlider)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mPersistent;
    bs << mAlwaysRun;
    bs << mValue;
    bs << mEaseType;
    bs << mEasePower;
END_SAVES

BEGIN_COPYS(FlowSlider)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowSlider, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mPersistent)
        COPY_MEMBER(mAlwaysRun)
        COPY_MEMBER(mValue)
        COPY_MEMBER(mEaseType)
        COPY_MEMBER(mEasePower)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowSlider)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mPersistent;
    d >> mAlwaysRun;
    bs >> mValue;
    bs >> (int &)mEaseType;
    bs >> mEasePower;
    bs >> (int &)mEaseFunc;
    UpdateEase();
END_LOADS

bool FlowSlider::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    if (IsRunning()) {
        MILO_NOTIFY(
            "FlowSlider re-entrance error, activated when already running, deactivating and aborting, check your logic"
        );
        Deactivate(false);
        return false;
    } else {
        PushDrivenProperties();
        if (mPersistent) {
            RegisterEvents(this);
            mEventsRegistered = true;
        }
        UpdateActivations();
        if (mAlwaysRun) {
            return true;
        } else {
            return IsRunning();
        }
    }
}

void FlowSlider::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    if (mEventsRegistered)
        PropertyEventListener::UnregisterEvents(this);
    FlowNode::Deactivate(b);
}

void FlowSlider::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        if (mEventsRegistered && unk58) {
            UnregisterEvents(this);
            mEventsRegistered = false;
            unk58 = false;
        } else if (mEventsRegistered) {
            return;
        }
        mFlowParent->ChildFinished(this);
    }
}

void FlowSlider::RequestStop() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStop();
}

void FlowSlider::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
}

bool FlowSlider::IsRunning() {
    if (mEventsRegistered)
        return true;
    return FlowNode::IsRunning();
}

void FlowSlider::UpdateIntensity() {
    PushDrivenProperties();
    UpdateActivations();
}

__declspec(noinline) void FlowSlider::UpdateEase() {
    EaseType e = mEaseType;
    MILO_ASSERT(e >= kEaseLinear && e <= kEaseQuarterHalfStairstep, 0x16b);
    mEaseFunc = GetEaseFunction(e);
}

void FlowSlider::ReActivate() {
    Timer timer;
    timer.Restart();
    UpdateIntensity();
    timer.Stop();
    FlowNode *flow;
    for (flow = GetTopFlow(); flow->GetParent() != nullptr;
         flow = flow->GetParent()->GetTopFlow()) {
    }
    Symbol sym = MakeString("%s: %s->%s", ClassName(), flow->Dir()->Name(), flow->Name());
    TheFlowMgr->AddEventTime(sym, timer.Ms());
}
