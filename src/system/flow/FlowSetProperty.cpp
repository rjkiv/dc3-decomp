#include "FlowSetProperty.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "math/Color.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include <cstdlib>

#pragma region PropertyTask

PropertyTask::PropertyTask(
    Hmx::Object *target,
    DataNode &n1,
    DataNode &n2,
    TaskUnits u,
    float f1,
    EaseType t,
    float f2,
    bool b,
    Hmx::Object *o2
)
    : unk2c(this), unk40(n1), unk48(n2), unk58(f1), unk5c(f2), unk60(b), unk64(this) {
    MILO_ASSERT(target, 0x4D);
    mEaseFunc = GetEaseFunctionForcedInline(t);
    FOREACH (it, target->Refs()) {
        Hmx::Object *owner = it->RefOwner();
        if (owner) {
            if (owner->ClassName() == StaticClassName()) {
                PropertyTask *cur = static_cast<PropertyTask *>(owner);
                if (cur->unk40.Array()->Size() == unk40.Array()->Size()) {
                    bool b3 = true;
                    for (int i = 0; i < unk40.Array()->Size(); i++) {
                        if (unk40.Array()->Node(i) != cur->unk40.Array()->Node(i)) {
                            b3 = false;
                            break;
                        }
                    }
                    if (b3) {
                        delete cur;
                        break;
                    }
                }
            }
        }
    }
    unk64 = o2;
    unk2c = target;
    unk50 = *unk2c->Property(unk40.Array());
    unk78 = unk50.Type();
    if (unk78 == kDataString) {
        unk50 = atoi(unk50.Str());
    }
    if (unk48.Type() == kDataString) {
        unk48 = atoi(unk48.Str());
    }
    TheTaskMgr.Start(this, u, 0);
}

PropertyTask::~PropertyTask() {}

bool PropertyTask::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &unk2c) {
        unk2c = to;
        if (!unk2c) {
            delete this;
        }
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

void PropertyTask::Poll(float f1) {
    if ((f1 / unk58) > 1) {
        ObjPtr<PropertyTask> task(this);
        task = this;
        SetProperty(unk48);
        if (task) {
            if (unk64) {
                static Message msg("on_anim_event", Symbol("ended"));
                Hmx::Object *obj = unk64;
                unk64 = nullptr;
                obj->Handle(msg, false);
            }
            delete this;
        }
    } else {
        float res = mEaseFunc(f1 / unk58, unk5c, 1);
        if (unk60) {
            Hmx::Color c;
            c.Unpack(unk50.Int());
            Hmx::Color c2;
            c2.Unpack(unk48.Int());
            Add(c, c2, c);
            int packed = c.Pack();
            unk2c->SetProperty(unk40.Array(), packed);
        } else {
            DataNode n(unk48);
            float nfloat1 = unk48.LiteralFloat();
            float nfloat2 = unk50.LiteralFloat();
            float math = (nfloat1 - nfloat2) * res + nfloat2;
            if (n.Type() == kDataInt || n.Type() == kDataString) {
                n = (int)math;
            } else {
                n = math;
            }
            SetProperty(n);
        }
    }
}

void PropertyTask::SetProperty(DataNode &n) {
    if (unk78 == kDataString) {
        unk2c->SetProperty(unk40.Array(), MakeString("%i", n));
    } else {
        unk2c->SetProperty(unk40.Array(), n);
    }
}

#pragma endregion
#pragma region FlowSetProperty

FlowSetProperty::FlowSetProperty()
    : PropertyEventListener(this), mTarget(this, nullptr), mValue(0), mPersistent(0),
      mRate(), mBlendTime(0), mChangePerUnit(0), unk_0xCC(this, nullptr), mEase(),
      mEasePower(2), unk_0xE8(0), mStopMode(kStopLastFrame) {}

FlowSetProperty::~FlowSetProperty() { TheFlowMgr->CancelCommand(this); }

bool FlowSetProperty::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &unk_0xCC) {
        unk_0xCC = nullptr;
        OnAnimEvent("interrupted");
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(FlowSetProperty)
    HANDLE_EXPR(get_property_path, mPropPath)
    HANDLE_ACTION(on_anim_event, OnAnimEvent(_msg->Sym(2)))
    HANDLE_EXPR(allow_blend, IsBlendable())
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSetProperty)
    SYNC_PROP_MODIFY(target, mTarget, OnTargetChanged())
    SYNC_PROP(value, mValue)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(rate, (int &)mRate)
    SYNC_PROP(blend_time, mBlendTime)
    SYNC_PROP(change_per_unit, mChangePerUnit)
    SYNC_PROP(ease, (int &)mEase)
    SYNC_PROP(ease_power, mEasePower)
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSetProperty)
    SAVE_REVS(3, 0);
    SAVE_SUPERCLASS(FlowNode);
    bs << mTarget;
    bs << mPropPath;
    if (mValue.Node().Type() == kDataObject) {
        bs << mValue.Node();
    } else {
        bs << mValue.Node().Type();
        bs << mValue.Node();
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

BEGIN_COPYS(FlowSetProperty)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSetProperty)
    BEGIN_COPYING_MEMBERS
        if (IsRunning()) {
            Deactivate(false);
        }
        UnregisterEvents(this);
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mPropPath)
        mValue = c->mValue.Node();
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

INIT_REVS(3, 0)

BEGIN_LOADS(FlowSetProperty)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (d.rev < 2) {
        mTarget = mTarget.LoadFromMainOrDir(bs);
    } else {
        d >> mTarget;
    }
    d >> mPropPath;
    if (d.rev < 1) {
        DataNode n;
        d >> n;
        mValue = n;
    } else if (d.rev == 2) {
        DataType t;
        d >> (int &)t;
        if (t == kDataObject) {
            Flow *flow = GetOwnerFlow();
            DirLoader *dl = flow->Loader();
            ObjectDir *dir = dl ? dl->ProxyDir() : flow->Dir();
            mValue = FlowNode::LoadObjectFromMainOrDir(bs, dir);
        } else {
            DataNode n;
            d >> n;
            mValue = n;
        }
    } else {
        DataType t;
        d >> (int &)t;
        if (t == kDataObject) {
            Flow *flow = GetOwnerFlow();
            if (!flow) {
                flow = dynamic_cast<Flow *>(this);
            }
            DirLoader *dl = flow->Loader();
            ObjectDir *dir = dl ? dl->ProxyDir() : flow->Dir();
            mValue = FlowNode::LoadObjectFromMainOrDir(bs, dir);
        } else {
            DataNode n;
            d >> n;
            mValue = n;
        }
    }
    d >> (BinStreamEnum<RndAnimatable::Rate> &)mRate;
    d >> mBlendTime;
    d >> mChangePerUnit;
    d >> (BinStreamEnum<EaseType> &)mEase;
    d >> mEasePower;
    d >> unk_0xE8;
    d >> mPersistent;
    d >> (BinStreamEnum<StopMode> &)mStopMode;
    GenerateAutoNames(this, true);
END_LOADS

bool FlowSetProperty::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (mTarget != nullptr) {
        if (mPropPath.Type() == kDataArray && mPropPath.Array()->Size() > 0) {
            if (mPersistent && !mEventsRegistered) {
                RegisterEvents(this);
            }
            const auto *value = GetDrivenEntry("value");
            if (value != nullptr) {
                mValue = mTarget->Property(mPropPath.Array(), true)->Evaluate();
            }
            PushDrivenProperties();

            if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
                FLOW_LOG("Setting Value on %s\n", mTarget->Name())
                mTarget->SetProperty(mPropPath.Array(), mValue.Node());
                return mEventsRegistered - 0;
            }
            if (mTarget->Property(mPropPath.Array(), true)->Evaluate()
                != mValue.Node().Evaluate()) {
                FLOW_LOG("Queueing\n")
                TheFlowMgr->QueueCommand(this, kQueue);
                return 1;
            }
        }
    }
    return mEventsRegistered;
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

void FlowSetProperty::ChildFinished(FlowNode *child) {
    FLOW_LOG("Child Finished of class: %s\n", child->ClassName());
    mRunningNodes.remove(child);
    if (mRunningNodes.empty() && !unk_0xCC && !mEventsRegistered) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowSetProperty::RequestStop() {
    FLOW_LOG("RequestStop\n");
    mRequestingStop = true;
    if (mStopMode == 0 || unk_0xCC == nullptr) {
        TheFlowMgr->QueueCommand(this, kIgnore);
    }
    FlowNode::RequestStop();
}

void FlowSetProperty::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    mRequestingStop = false;
    if (mStopMode != 0) {
        TheFlowMgr->QueueCommand(this, kQueue);
    }
    FlowNode::RequestStopCancel();
}

void FlowSetProperty::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (IsRunning()) {
        if (qs == kIgnore) {
            FLOW_LOG("RequestStop: Stopping\n");
            if (mEventsRegistered) {
                UnregisterEvents(this);
            }
            if (unk_0xCC) {
                Task *task = unk_0xCC;
                unk_0xCC = nullptr;
                delete task;
            }
            mFlowParent->ChildFinished(this);
        }
    } else if (qs == kQueue) {
        float time = mBlendTime;
        if (mChangePerUnit && !unk_0xE8) {
            const DataNode *prop = mTarget->Property(mPropPath.Array());
            if (prop->Type() == kDataFloat) {
                time = Abs((mValue.Node().Float() - prop->Float()) / mChangePerUnit);
            } else if (prop->Type() == kDataInt) {
                time = Abs((float)(mValue.Node().Int() - prop->Int()) / mChangePerUnit);
            } else {
                StackString<32> str;
                mValue.Node().Print(str, false, 0);
                MILO_NOTIFY(
                    "%s has bad property %s for %s",
                    PathName(this),
                    str,
                    PrintPropertyPath(mPropPath.Array())
                );
                time = 0;
            }
        }
        if (time > 0) {
            static TaskUnits sRateUnits[] = {
                kTaskSeconds, kTaskBeats, kTaskUISeconds, kTaskBeats, kTaskTutorialSeconds
            };

            Hmx::Object *target = mTarget;
            DataNode val = mValue.Node();
            FLOW_LOG("Spawning Ramp Task on %s\n", target->Name());
            unk_0xCC = new PropertyTask(
                target,
                mPropPath,
                val,
                sRateUnits[mRate],
                time,
                mEase,
                mEasePower,
                unk_0xE8,
                this
            );
        } else {
            FLOW_LOG("Setting Value on %s\n", mTarget->Name());
            mTarget->SetProperty(mPropPath.Array(), mValue.Node());
            if (!mEventsRegistered) {
                FLOW_LOG("Releasing\n");
                mFlowParent->ChildFinished(this);
            }
        }
    } else if (qs == kIgnore) {
        mFlowParent->ChildFinished(this);
    }
}

bool FlowSetProperty::IsRunning() {
    if (!mEventsRegistered) {
        if (mRunningNodes.size() == 0) {
            return unk_0xCC.Ptr();
        }
    }
    return true;
}

void FlowSetProperty::MiloPreRun() {
    UnregisterEvents(this);
    GenerateAutoNames(this, 1);
    FlowNode::MiloPreRun();
}

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

void FlowSetProperty::UpdateIntensity(void) {
    if (mPersistent)
        Activate();
}

void FlowSetProperty::OnTargetChanged(void) {
    if (mTarget != nullptr) {
        if (mPropPath.Type() == kDataArray && mPropPath.Array()->Size() > 0) {
            const DataNode *props = mTarget->Property(mPropPath.Array(), false);
            if (props == nullptr)
                mPropPath = 0;
        }
    }
    if (mTarget == nullptr) {
        mPropPath = 0;
    }
}

bool FlowSetProperty::IsBlendable(void) {
    if (mTarget == nullptr)
        return false;
    if (!mPropPath.NotNull())
        return false;
    const DataNode *props = mTarget->Property(mPropPath.Array(), false);
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

void FlowSetProperty::ReActivate() {
    FLOW_LOG("Reactivate\n");
    Timer t;
    t.Restart();
    PushDrivenProperties();
    if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
        FLOW_LOG("Setting Value on %s\n", mTarget->Name())
        mTarget->SetProperty(mPropPath.Array(), mValue.Node());
        return;
    }
    if (mTarget->Property(mPropPath.Array(), true)->Evaluate()
        != mValue.Node().Evaluate()) {
        FLOW_LOG("Queueing\n")
        TheFlowMgr->QueueCommand(this, kQueue);
    }
    t.Stop();

    FlowNode *flow;
    for (flow = GetTopFlow(); flow->GetParent() != nullptr;
         flow = flow->GetParent()->GetTopFlow()) {
    }
    Symbol sym = MakeString("%s: %s->%s", ClassName(), flow->Dir()->Name(), flow->Name());
    TheFlowMgr->AddEventTime(sym, t.Ms());
}

void FlowSetProperty::OnAnimEvent(Symbol) {
    FLOW_LOG("PropertyRampEnded");
    unk_0xCC = nullptr;
    if (mRunningNodes.empty() && !mEventsRegistered) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

#pragma endregion
