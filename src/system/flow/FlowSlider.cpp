#include "flow/FlowSlider.h"
#include "FlowValueCase.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/PropertyEventListener.h"
#include "flow/Flow.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "os/Debug.h"

bool SliderChildSort(FlowNode *n1, FlowNode *n2) {
    FlowValueCase *v1 = (FlowValueCase *)n1;
    FlowValueCase *v2 = (FlowValueCase *)n2;
    return v1->GetValue() < v2->GetValue();
}

FlowSlider::FlowSlider()
    : PropertyEventListener(this), mPersistent(1), mAlwaysRun(0), mValue(0),
      mEase(kEasePolyOut), mEasePower(2) {
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
    SYNC_PROP_MODIFY(ease, (int &)mEase, UpdateEase())
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
    bs << mEase;
    bs << mEasePower;
END_SAVES

BEGIN_COPYS(FlowSlider)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowSlider, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mPersistent)
        COPY_MEMBER(mAlwaysRun)
        COPY_MEMBER(mValue)
        COPY_MEMBER(mEase)
        COPY_MEMBER(mEasePower)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowSlider)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mPersistent;
    d >> mAlwaysRun;
    d >> mValue;
    d >> (int &)mEase;
    d >> mEasePower;
    GenerateAutoNames(this, true);
    mChildNodes.sort(SliderChildSort);
    UpdateEase();
END_LOADS

bool FlowSlider::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
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
            mRegistered = true;
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
    if (mRegistered)
        PropertyEventListener::UnregisterEvents(this);
    FlowNode::Deactivate(b);
}

void FlowSlider::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty()) {
        if (mRegistered && mRequestingStop) {
            UnregisterEvents(this);
            mRegistered = false;
            mRequestingStop = false;
        } else if (mRegistered) {
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
    if (mRegistered)
        return true;
    return FlowNode::IsRunning();
}

void FlowSlider::UpdateIntensity() {
    PushDrivenProperties();
    UpdateActivations();
}

void FlowSlider::UpdateEase() { pt2EaseFuncion = GetEaseFunction(mEase); }

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

// void FlowSlider::UpdateActivations() {
//     float old = FlowNode::sIntensity;
//     auto it = mChildNodes.begin();
//     auto it8 = mChildNodes.begin();
//     auto it6 = mChildNodes.begin();
//     for (; it != mChildNodes.end(); ++it) {
//         it6++;
//         FlowValueCase *cur = static_cast<FlowValueCase *>(*it);

//         // yeah idk man
//         if (it6 != mChildNodes.end()) {
//         } else {
//         }
//     }
//     sIntensity = old;
// }
