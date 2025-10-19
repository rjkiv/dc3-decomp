#include "FlowSetProperty.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/MakeString.h"

FlowSetProperty::~FlowSetProperty() { TheFlowMgr->CancelCommand(this); }

FlowSetProperty::FlowSetProperty()
    : PropertyEventListener(this), mTarget(this, nullptr), unk_0x98(0), mValue(0),
      mPersistent(0), mRate(0), mBlendTime(0), mChangePerUnit(0), unk_0xCC(this, nullptr),
      mEase(0), mEasePower(2), unk_0xE8(0), mStopMode(1) {}

PropertyTask::PropertyTask(
    Hmx::Object *,
    DataNode &,
    DataNode &,
    TaskUnits,
    float,
    EaseType t,
    float,
    bool,
    Hmx::Object *
) {
    mEaseFunc = GetEaseFunction(t);
}

BEGIN_PROPSYNCS(FlowSetProperty)
    SYNC_PROP_MODIFY(target, mTarget, OnTargetChanged())
    SYNC_PROP(value, mValue)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(rate, mRate)
    SYNC_PROP(blend_time, mBlendTime)
    SYNC_PROP(change_per_unit, mChangePerUnit)
    SYNC_PROP(ease, mEase)
    SYNC_PROP(ease_power, mEasePower)
    SYNC_PROP(stop_mode, mStopMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_HANDLERS(FlowSetProperty)
    HANDLE_EXPR(get_property_path, unk_0x98)
    HANDLE_ACTION(on_anim_event, OnAnimEvent(_msg->Sym(2)))
    HANDLE_EXPR(allow_blend, IsBlendable())
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

void FlowSetProperty::OnTargetChanged(void) {
    if (mTarget != nullptr) {
        if (unk_0x98.Type() == kDataArray && unk_0x98.Array()->Size() > 0) {
            const DataNode *props = mTarget->Property(unk_0x98.Array(), false);
            if (props == nullptr)
                unk_0x98 = 0;
        }
    }
    if (mTarget == nullptr) {
        unk_0x98 = 0;
    }
}

bool FlowSetProperty::IsBlendable(void) {
    if (mTarget == nullptr)
        return false;
    if (!unk_0x98.NotNull())
        return false;
    const DataNode *props = mTarget->Property(unk_0x98.Array(), false);
    if (props == nullptr)
        return false;
    if (props->Type() == kDataInt || props->Type() == kDataFloat)
        return true;
    DrivenPropertyEntry *dpe = GetDrivenEntry("value");
    if (dpe == nullptr)
        return false;
    if (dpe->Empty())
        return false;
    return true;
}

BEGIN_COPYS(FlowSetProperty)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSetProperty)
    BEGIN_COPYING_MEMBERS
        UnregisterEvents(this);

        COPY_MEMBER(mRate)
        COPY_MEMBER(mBlendTime)
        COPY_MEMBER(mChangePerUnit)
        COPY_MEMBER(mEase)
        COPY_MEMBER(mEasePower)
        COPY_MEMBER(unk_0xE8)
        COPY_MEMBER(mPersistent)
        COPY_MEMBER(mStopMode)
        GenerateAutoNames(this, true);
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowSetProperty)
END_LOADS

void FlowSetProperty::MoveIntoDir(ObjectDir *r4, ObjectDir *r5) {
    FlowNode::MoveIntoDir(r4, r5);
    if (mTarget == r5) {
        Hmx::Object *obj;
        if (r4 == nullptr) {
            obj = nullptr;
        } else {
            obj = r4;
        }
        mTarget = obj;
    }
}

bool FlowSetProperty::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    if (mTarget != nullptr) {
        if (unk_0x98.Type() == kDataArray && unk_0x98.Array()->Size() > 0) {
            if (mPersistent && !unk14) {
                RegisterEvents(this);
            }
            const auto *value = GetDrivenEntry("value");
            if (value != nullptr) {
                mValue = mTarget->Property(unk_0x98.Array(), true)->Evaluate();
            }
            PushDrivenProperties();

            if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
                FLOW_LOG("Setting Value on %s\n", mTarget->Name())
                mTarget->SetProperty(unk_0x98.Array(), mValue.Node());
                return unk14 - 0;
            }
            if (mTarget->Property(unk_0x98.Array(), true)->Evaluate()
                != mValue.Node().Evaluate()) {
                FLOW_LOG("Queueing\n")
                TheFlowMgr->QueueCommand(this, kQueue);
                return 1;
            }
        }
    }

    return unk14;
}

void FlowSetProperty::ReActivate() {
    FLOW_LOG("Reactivate\n");
    Timer t;
    t.Restart();
    PushDrivenProperties();
    if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
        FLOW_LOG("Setting Value on %s\n", mTarget->Name())
        mTarget->SetProperty(unk_0x98.Array(), mValue.Node());
        return;
    }
    if (mTarget->Property(unk_0x98.Array(), true)->Evaluate()
        != mValue.Node().Evaluate()) {
        FLOW_LOG("Queueing\n")
        TheFlowMgr->QueueCommand(this, kQueue);
    }
    t.Stop();
    for (FlowNode *node = this;; node = node->GetParent()) {
        node = node->GetTopFlow();
        if (node->GetParent() == nullptr)
            break;
    }
    TheFlowMgr->AddEventTime("penis", t.Ms()); // i have no fucking clue what's going on
                                               // at the end of this func. penis
}

void FlowSetProperty::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n");
    if (!IsRunning() || qs == kIgnore) {
        FLOW_LOG("RequestStop: Stopping\n");
        UnregisterEvents(this);
    }
}

void FlowSetProperty::ChildFinished(FlowNode *child) {
    FLOW_LOG("Child Finished of class: %s\n", child->ClassName());
}

bool FlowSetProperty::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == static_cast<ObjRef *>(&unk_0xCC)) {
        unk_0xCC = nullptr;
        OnAnimEvent("interrupted");
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_SAVES(FlowSetProperty)
    SAVE_REVS(4, 0) SAVE_SUPERCLASS(FlowNode) bs << mTarget;
    bs << unk_0x98;
    if (!mValue.Node()) {
    }

    bs << mRate;
    bs << mBlendTime;
    bs << mChangePerUnit;
    bs << mEase;
    bs << mEasePower;
    bs << unk_0xE8;
    bs << mPersistent;
    bs << mStopMode;
END_SAVES

void FlowSetProperty::OnAnimEvent(Symbol) {
    FLOW_LOG("PropertyRampEnded");
    FLOW_LOG("Timed Release From Parent \n");
    Timer t;
    t.Reset();
}

void FlowSetProperty::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    if (!b) {
        UnregisterEvents(this);
    }
    TheFlowMgr->CancelCommand(this);
    if (unk_0xCC != nullptr) {
        auto *idiot = unk_0xCC.Ptr();
        unk_0xCC = nullptr;
        delete idiot;
    }
    FlowNode::Deactivate(b);
}

bool FlowSetProperty::IsRunning() {
    if (!unk14) {
        if (mRunningNodes.size() == 0) {
            return unk_0xCC.Ptr();
        }
    }
    return true;
}

void FlowSetProperty::RequestStop() {
    FLOW_LOG("RequestStop\n");
    unk58 = true;
    if (mStopMode == 0 || unk_0xCC == nullptr) {
        TheFlowMgr->QueueCommand(this, kIgnore);
    }
    FlowNode::RequestStop();
}

void FlowSetProperty::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    unk58 = false;
    if (mStopMode != 0) {
        TheFlowMgr->QueueCommand(this, kQueue);
    }
    FlowNode::RequestStopCancel();
}

void FlowSetProperty::MiloPreRun() {
    UnregisterEvents(this);
    GenerateAutoNames(this, 1);
    FlowNode::MiloPreRun();
}

void FlowSetProperty::UpdateIntensity(void) {
    if (mPersistent)
        Activate();
}
