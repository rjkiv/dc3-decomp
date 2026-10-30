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
    : mTarget(this), mProp(n1), mTargetValue(n2), mTime(f1), mEasePower(f2),
      mUseColorInterp(b), mEventReciever(this) {
    MILO_ASSERT(target, 0x4D);
    pt2EaseFuncion = GetEaseFunction(t);
    FOREACH_OBJREF (it, target) {
        Hmx::Object *owner = it->RefOwner();
        if (owner) {
            if (owner->ClassName() == StaticClassName()) {
                PropertyTask *cur = static_cast<PropertyTask *>(owner);
                if (cur->mProp.Array()->Size() == mProp.Array()->Size()) {
                    bool b3 = true;
                    for (int i = 0; i < mProp.Array()->Size(); i++) {
                        if (mProp.Array()->Node(i) != cur->mProp.Array()->Node(i)) {
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
    mEventReciever = o2;
    mTarget = target;
    mStartValue = *mTarget->Property(mProp.Array());
    mTargetType = mStartValue.Type();
    if (mTargetType == kDataString) {
        mStartValue = atoi(mStartValue.Str());
    }
    if (mTargetValue.Type() == kDataString) {
        mTargetValue = atoi(mTargetValue.Str());
    }
    TheTaskMgr.Start(this, u, 0);
}

PropertyTask::~PropertyTask() {}

bool PropertyTask::Replace(ObjRef *from, Hmx::Object *to) {
    auto &ref = mTarget;
    if (from == &ref) {
        ref = to;
        if (!ref) {
            delete this;
        }
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

void PropertyTask::Poll(float f1) {
    if ((f1 / mTime) > 1) {
        ObjPtr<PropertyTask> task(this);
        task = this;
        SetProperty(mTargetValue);
        if (task) {
            if (mEventReciever) {
                static Message msg("on_anim_event", Symbol("ended"));
                Hmx::Object *obj = mEventReciever;
                mEventReciever = nullptr;
                obj->Handle(msg, false);
            }
            delete this;
        }
    } else {
        float res = pt2EaseFuncion(f1 / mTime, mEasePower, 1);
        if (mUseColorInterp) {
            Hmx::Color c;
            c.UnpackRgb(mStartValue.Int());
            Hmx::Color c2;
            c2.UnpackRgb(mTargetValue.Int());
            Add(c, c2, c);
            int packed = c.PackRgb();
            mTarget->SetProperty(mProp.Array(), packed);
        } else {
            DataNode n(mTargetValue);
            float nfloat1 = mTargetValue.LiteralFloat();
            float nfloat2 = mStartValue.LiteralFloat();
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
    if (mTargetType == kDataString) {
        mTarget->SetProperty(mProp.Array(), MakeString("%i", n));
    } else {
        mTarget->SetProperty(mProp.Array(), n);
    }
}

#pragma endregion
#pragma region FlowSetProperty

FlowSetProperty::FlowSetProperty()
    : PropertyEventListener(this), mTargetObject(this, nullptr), mValue(0),
      mRegisterPEPs(0), mRate(), mBlendTime(0), mChangePerUnit(0),
      mSpawnedTask(this, nullptr), mEase(), mEasePower(2), mUseColorInterp(0),
      mStopMode(kStopLastFrame) {}

FlowSetProperty::~FlowSetProperty() { TheFlowMgr->CancelCommand(this); }

bool FlowSetProperty::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mSpawnedTask) {
        mSpawnedTask = nullptr;
        OnAnimEvent("interrupted");
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(FlowSetProperty)
    HANDLE_EXPR(get_property_path, mProperty)
    HANDLE_ACTION(on_anim_event, OnAnimEvent(_msg->Sym(2)))
    HANDLE_EXPR(allow_blend, IsBlendable())
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSetProperty)
    SYNC_PROP_MODIFY(target, mTargetObject, OnTargetChanged())
    SYNC_PROP(value, mValue)
    SYNC_PROP(persistent, mRegisterPEPs)
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
    bs << mTargetObject;
    bs << mProperty;
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
    bs << mUseColorInterp;
    bs << mRegisterPEPs;
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
        COPY_MEMBER(mTargetObject)
        COPY_MEMBER(mProperty)
        mValue = c->mValue.Node();
        COPY_MEMBER(mRate)
        COPY_MEMBER(mBlendTime)
        COPY_MEMBER(mChangePerUnit)
        COPY_MEMBER(mEase)
        COPY_MEMBER(mEasePower)
        COPY_MEMBER(mUseColorInterp)
        COPY_MEMBER(mRegisterPEPs)
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
        mTargetObject = mTargetObject.LoadFromMainOrDir(bs);
    } else {
        d >> mTargetObject;
    }
    d >> mProperty;
    if (d.rev < 1) {
        DataNode n;
        d >> n;
        mValue = n;
    } else if (d.rev == 2) {
        DataType t;
        d >> (int &)t;
        if (t == kDataObject) {
            Flow *flow = GetOwnerFlow();
            mValue = FlowNode::LoadObjectFromMainOrDir(bs, flow->LoadingDir());
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
            mValue = FlowNode::LoadObjectFromMainOrDir(bs, flow->LoadingDir());
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
    d >> mUseColorInterp;
    d >> mRegisterPEPs;
    d >> (BinStreamEnum<StopMode> &)mStopMode;
    GenerateAutoNames(this, true);
END_LOADS

bool FlowSetProperty::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (mTargetObject != nullptr) {
        if (mProperty.Type() == kDataArray && mProperty.Array()->Size() > 0) {
            if (mRegisterPEPs && !mRegistered) {
                RegisterEvents(this);
            }
            const auto *value = GetDrivenEntry("value");
            if (value != nullptr) {
                mValue = mTargetObject->Property(mProperty.Array(), true)->Evaluate();
            }
            PushDrivenProperties();

            if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
                FLOW_LOG("Setting Value on %s\n", mTargetObject->Name())
                mTargetObject->SetProperty(mProperty.Array(), mValue.Node());
                return mRegistered - 0;
            }
            if (mTargetObject->Property(mProperty.Array(), true)->Evaluate()
                != mValue.Node().Evaluate()) {
                FLOW_LOG("Queueing\n")
                TheFlowMgr->QueueCommand(this, kStarting);
                return 1;
            }
        }
    }
    return mRegistered;
}

void FlowSetProperty::Deactivate(bool b) {
    FLOW_LOG("Deactivated\n");
    if (!b) {
        UnregisterEvents(this);
    }
    TheFlowMgr->CancelCommand(this);
    if (mSpawnedTask != nullptr) {
        auto *idiot = mSpawnedTask.Ptr();
        mSpawnedTask = nullptr;
        delete idiot;
    }
    FlowNode::Deactivate(b);
}

void FlowSetProperty::ChildFinished(FlowNode *child) {
    FLOW_LOG("Child Finished of class: %s\n", child->ClassName());
    mRunningNodes.remove(child);
    if (mRunningNodes.empty() && !mSpawnedTask && !mRegistered) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowSetProperty::RequestStop() {
    FLOW_LOG("RequestStop\n");
    mRequestingStop = true;
    if (mStopMode == 0 || mSpawnedTask == nullptr) {
        TheFlowMgr->QueueCommand(this, kRequestStop);
    }
    FlowNode::RequestStop();
}

void FlowSetProperty::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    mRequestingStop = false;
    if (mStopMode != 0) {
        TheFlowMgr->QueueCommand(this, kStarting);
    }
    FlowNode::RequestStopCancel();
}

void FlowSetProperty::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (IsRunning()) {
        if (qs == kRequestStop) {
            FLOW_LOG("RequestStop: Stopping\n");
            if (mRegistered) {
                UnregisterEvents(this);
            }
            if (mSpawnedTask) {
                Task *task = mSpawnedTask;
                mSpawnedTask = nullptr;
                delete task;
            }
            mFlowParent->ChildFinished(this);
        }
    } else if (qs == kStarting) {
        float time = mBlendTime;
        if (mChangePerUnit && !mUseColorInterp) {
            const DataNode *prop = mTargetObject->Property(mProperty.Array());
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
                    PrintPropertyPath(mProperty.Array())
                );
                time = 0;
            }
        }
        if (time > 0) {
            static TaskUnits sRateUnits[] = {
                kTaskSeconds, kTaskBeats, kTaskUISeconds, kTaskBeats, kTaskTutorialSeconds
            };

            Hmx::Object *target = mTargetObject;
            DataNode val = mValue.Node();
            FLOW_LOG("Spawning Ramp Task on %s\n", target->Name());
            mSpawnedTask = new PropertyTask(
                target,
                mProperty,
                val,
                sRateUnits[mRate],
                time,
                mEase,
                mEasePower,
                mUseColorInterp,
                this
            );
        } else {
            FLOW_LOG("Setting Value on %s\n", mTargetObject->Name());
            mTargetObject->SetProperty(mProperty.Array(), mValue.Node());
            if (!mRegistered) {
                FLOW_LOG("Releasing\n");
                mFlowParent->ChildFinished(this);
            }
        }
    } else if (qs == kRequestStop) {
        mFlowParent->ChildFinished(this);
    }
}

bool FlowSetProperty::IsRunning() {
    if (!mRegistered) {
        if (mRunningNodes.size() == 0) {
            return mSpawnedTask.Ptr();
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
    if (mTargetObject == r5) {
        Hmx::Object *obj;
        if (r4 == nullptr) {
            obj = nullptr;
        } else {
            obj = r4;
        }
        mTargetObject = obj;
    }
}

void FlowSetProperty::UpdateIntensity(void) {
    if (mRegisterPEPs)
        Activate();
}

void FlowSetProperty::OnTargetChanged(void) {
    if (mTargetObject != nullptr) {
        if (mProperty.Type() == kDataArray && mProperty.Array()->Size() > 0) {
            const DataNode *props = mTargetObject->Property(mProperty.Array(), false);
            if (props == nullptr)
                mProperty = 0;
        }
    }
    if (mTargetObject == nullptr) {
        mProperty = 0;
    }
}

bool FlowSetProperty::IsBlendable(void) {
    if (mTargetObject) {
        if (mProperty.NotNull()) {
            const DataNode *props = mTargetObject->Property(mProperty.Array(), false);
            if (props) {
                if (props->Type() == kDataInt || props->Type() == kDataFloat) {
                    return true;
                }
                DrivenPropertyEntry *dpe = GetDrivenEntry("value");
                if (dpe) {
                    if (!dpe->mMathOps.empty()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void FlowSetProperty::ReActivate() {
    FLOW_LOG("Reactivate\n");
    Timer t;
    t.Restart();
    PushDrivenProperties();
    if (mBlendTime == 0.0f && mChangePerUnit == 0.0f) {
        FLOW_LOG("Setting Value on %s\n", mTargetObject->Name())
        mTargetObject->SetProperty(mProperty.Array(), mValue.Node());
    } else if (
        mTargetObject->Property(mProperty.Array(), true)->Evaluate()
        != mValue.Node().Evaluate()
    ) {
        FLOW_LOG("Queueing\n")
        TheFlowMgr->QueueCommand(this, kStarting);
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
    mSpawnedTask = nullptr;
    if (mRunningNodes.empty() && !mRegistered) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

#pragma endregion
