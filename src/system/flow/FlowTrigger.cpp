#include "flow/FlowTrigger.h"
#include "flow/FlowNode.h"
#include "flow/FlowQueueable.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

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

FlowTrigger::PropTriggerDefn::PropTriggerDefn(Hmx::Object *owner) : mProvider(owner) {
    unk20 = 0;
}
