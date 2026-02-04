#include "flow/FlowTrigger.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

FlowTrigger::FlowTrigger()
    : mEventProvider(this), mTriggerProperties(this), mStopProperties(this), mHardStop(0),
      unkb1(0) {}
FlowTrigger::~FlowTrigger() {}

BEGIN_HANDLERS(FlowTrigger)
    HANDLE_ACTION(activate, ActivateWithParams(nullptr, _msg))
    HANDLE_ACTION(deactivate, Deactivate(false))
    HANDLE_ACTION(request_stop, RequestStop())
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(FlowTrigger::PropTriggerDefn)
    SYNC_PROP(provider, o.mProvider)
    SYNC_PROP_SET(path, o.GetPathDisplay(nullptr), )
END_CUSTOM_PROPSYNC

#define SYNC_PROP_TRIGGERS(s, member)                                                    \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (!(_op & (kPropSize | kPropGet))) {                                       \
                UnregisterEvents();                                                      \
            }                                                                            \
            if (PropSync(member, _val, _prop, _i + 1, _op)) {                            \
                if (!(_op & (kPropSize | kPropGet))) {                                   \
                    RegisterEvents();                                                    \
                }                                                                        \
                return true;                                                             \
            } else {                                                                     \
                return false;                                                            \
            }                                                                            \
        }                                                                                \
    }

BEGIN_PROPSYNCS(FlowTrigger)
    SYNC_PROP(event_provider, mEventProvider)
    SYNC_PROP_TRIGGERS(trigger_events, mTriggerEvents)
    SYNC_PROP_TRIGGERS(stop_events, mStopEvents)
    SYNC_PROP_TRIGGERS(trigger_properties, mTriggerProperties)
    SYNC_PROP_TRIGGERS(stop_properties, mStopProperties)
    SYNC_PROP(hard_stop, mHardStop)
    SYNC_SUPERCLASS(FlowQueueable)
END_PROPSYNCS

BEGIN_SAVES(FlowTrigger)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowQueueable)
    bs << mEventProvider;
    bs << mTriggerEvents;
    bs << mStopEvents;
    bs << mHardStop;
    bs << mTriggerProperties;
    bs << mStopProperties;
END_SAVES

BEGIN_COPYS(FlowTrigger)
    COPY_SUPERCLASS(FlowQueueable)
    CREATE_COPY(FlowTrigger)
    BEGIN_COPYING_MEMBERS
        UnregisterEvents();
        COPY_MEMBER(mTriggerEvents)
        COPY_MEMBER(mEventProvider)
        COPY_MEMBER(mStopEvents)
        COPY_MEMBER(mHardStop)
        COPY_MEMBER(mTriggerProperties)
        COPY_MEMBER(mStopProperties)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(FlowTrigger)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    FlowQueueable::Load(bs);
    UnregisterEvents();
    if (d.rev < 1) {
        bool load;
        d >> load;
        if (load) {
            mEventProvider = LoadObjectFromMainOrDir(bs, Dir());
        }
    } else {
        bs >> mEventProvider;
    }
    d >> mTriggerEvents;
    d >> mStopEvents;
    d >> mHardStop;
    if (d.rev > 0) {
        d >> mTriggerProperties;
        d >> mStopProperties;
    } else {
        FOREACH (it, mTriggerEvents) {
            String cur = it->Str();
            if (cur.contains("on_") && cur.contains("_change")) {
                cur.erase(cur.length() - 7, 7);
                cur.erase(0, 3);
                PropTriggerDefn defn(this);
                defn.mProvider = mEventProvider;
                DataArrayPtr ptr(new DataArray(1));
                ptr->Node(0) = Symbol(cur.c_str());
                defn.unk20 = ptr;
                mTriggerProperties.push_back(defn);
                mTriggerEvents.erase(it);
            }
        }
    }
END_LOADS

bool FlowTrigger::ActivateWithParams(Hmx::Object *o, DataArray *a) {
    FLOW_LOG("Trigger Activated \n");
    Timer timer;
    timer.Restart();
    PushDrivenProperties();
    if (GetOwnerFlow()) {
        GetOwnerFlow()->ApplyParams(a, this);
    }
    bool ret = FlowQueueable::Activate(o);
    Symbol sym("anon");
    if (mTriggerEvents.size() != 0) {
        if (mTriggerEvents.size() > 1) {
            sym = MakeString("Event:%s...", mTriggerEvents.front());
            goto flow_event;
        } else {
            sym = MakeString("Event:%s", mTriggerEvents.front());
            goto flow_event;
        }
    }
    if (mTriggerProperties.empty()) {
        goto flow_event;
    }
    if (mTriggerProperties.size() == 0) {
        goto flow_event;
    }
    if (!mTriggerProperties.empty()) {
        PropTriggerDefn &defn = mTriggerProperties.front();
        if (mTriggerProperties.size() > 1) {
            sym = MakeString(
                "PropChange:%s->%s...", defn.mProvider->Name(), defn.GetPathDisplay(0)
            );
        } else {
            sym = MakeString(
                "PropChange:%s->%s", defn.mProvider->Name(), defn.GetPathDisplay(0)
            );
        }
    }
flow_event:
    timer.Stop();
    TheFlowMgr->AddEventTime(sym, timer.Ms());
    return ret;
}

FlowTrigger::PropTriggerDefn::PropTriggerDefn(Hmx::Object *owner) : mProvider(owner) {
    unk20 = 0;
}

DataNode FlowTrigger::PropTriggerDefn::GetPathDisplay(DataArray *a) {
    if (mProvider && unk20.Type() == kDataArray) {
        if (unk20.Array()->Size() != 0) {
            String str;
            unk20.Print(str, true, 0);
            return MakeString("%s->%s", mProvider->Name(), str.c_str());
        }
    }
    return "<none>";
}

Hmx::Object *FlowTrigger::GetEventProvider() {
    if (mEventProvider) {
        return mEventProvider;
    } else {
        ObjectDir *dir;
        if (GetOwnerFlow()) {
            dir = GetTopFlow()->Dir();
        } else {
            dir = Dir();
        }
        return dir;
    }
}

DataArray *FlowTrigger::GetEventEditorDef(Symbol s) {
    DataArray *eval;
    if (mEventProvider && mEventProvider->TypeDef()
        && mEventProvider->TypeDef()->FindArray("supported_events", false)) {
        eval = mEventProvider->TypeDef()->FindArray("supported_events", true);
    } else if (mEventProvider
               && mEventProvider->ObjectDef(gNullStr)->FindArray(
                   "supported_events", false
               )) {
        eval = mEventProvider->ObjectDef(gNullStr)->FindArray("supported_events", true);
    } else {
        Flow *owner = GetOwnerFlow();
        if (owner->TypeDef() && owner->TypeDef()->FindArray("supported_events", false)) {
            eval = owner->TypeDef()->FindArray("supported_events", true);
        } else if (owner->ObjectDef(gNullStr)->FindArray("supported_events", false)) {
            eval = owner->ObjectDef(gNullStr)->FindArray("supported_events", true);
        } else
            return nullptr;
    }
    DataArray *a = eval->Array(1);
    if (a) {
        a = a->FindArray(s, false);
        if (a)
            return a;
    }
    return nullptr;
}

void FlowTrigger::RegisterEvents() {
    if (unkb1) {
        FLOW_LOG("Registering Events\n");
        static Symbol activate("activate");
        static Symbol deactivate("deactivate");
        static Symbol request_stop("request_stop");
        Hmx::Object *prov = GetEventProvider();
        if (prov) {
            FOREACH (it, mTriggerEvents) {
                prov->AddSink(this, *it, activate, kHandle, false);
            }
            if (mHardStop) {
                FOREACH (it, mStopEvents) {
                    prov->AddSink(this, *it, deactivate, kHandle, false);
                }
            } else {
                FOREACH (it, mStopEvents) {
                    prov->AddSink(this, *it, request_stop, kHandle, false);
                }
            }
        }
        FOREACH (it, mTriggerProperties) {
            Hmx::Object *prov = it->mProvider;
            if (prov && it->unk20.Type() == kDataArray) {
                prov->AddPropertySink(this, it->unk20.Array(), activate);
            }
        }
        if (mHardStop) {
            FOREACH (it, mStopProperties) {
                Hmx::Object *prov = it->mProvider;
                if (prov && it->unk20.Type() == kDataArray) {
                    prov->AddPropertySink(this, it->unk20.Array(), deactivate);
                }
            }
        } else {
            FOREACH (it, mStopProperties) {
                Hmx::Object *prov = it->mProvider;
                if (prov && it->unk20.Type() == kDataArray) {
                    prov->AddPropertySink(this, it->unk20.Array(), request_stop);
                }
            }
        }
    }
}

void FlowTrigger::UnregisterEvents() {
    FLOW_LOG("Unregistering Events\n");
    Hmx::Object *prov = mEventProvider;
    if (prov) {
        FOREACH (it, mTriggerEvents) {
            prov->RemoveSink(this, *it);
        }
        FOREACH (it, mStopEvents) {
            prov->RemoveSink(this, *it);
        }
    }
    FOREACH (it, mTriggerProperties) {
        Hmx::Object *prov = it->mProvider;
        if (prov && it->unk20.Type() == kDataArray) {
            prov->RemovePropertySink(this, it->unk20.Array());
        }
    }
    FOREACH (it, mStopProperties) {
        Hmx::Object *prov = it->mProvider;
        if (prov && it->unk20.Type() == kDataArray) {
            prov->RemovePropertySink(this, it->unk20.Array());
        }
    }
}
