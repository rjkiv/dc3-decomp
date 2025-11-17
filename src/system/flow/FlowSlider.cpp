#include "flow/FlowSlider.h"
#include "flow/FlowNode.h"
#include "flow/PropertyEventListener.h"
#include "math/Easing.h"
#include "obj/Object.h"

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

BEGIN_SAVES(FlowSlider)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mPersistent;
    bs << mAlwaysRun;
    bs << mValue;
    bs << mEaseType;
    bs << mEasePower;
END_SAVES

void FlowSlider::RequestStop() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStop();
}

void FlowSlider::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
}

void FlowSlider::UpdateEase() { mEaseFunc = GetEaseFunction(mEaseType); }

bool FlowSlider::IsRunning() {
    if (unk14)
        return true;
    return !mRunningNodes.empty();
}

void FlowSlider::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    if (unk14)
        PropertyEventListener::UnregisterEvents(this);
    FlowNode::Deactivate(b);
}
