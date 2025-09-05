#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"

Flow::Flow()
    : mDynamicProperties(this), mFlowLabels(this), mFlowOutPorts(this), mObjects(this),
      unk170(0), mPrivate(1), mHardStop(0), unk178(0) {}

Flow::~Flow() {
    FlowQueueable::Deactivate(true);
    TheFlowMgr->CancelCommand(this);
}

BEGIN_HANDLERS(Flow)
    HANDLE_ACTION(activate, Activate(false))
    HANDLE_ACTION(on_reflected_property_changed, OnReflectedPropertyChanged(_msg))
    HANDLE_ACTION(on_internal_property_changed, OnInternalPropertyChanged(_msg))
    HANDLE_ACTION(request_stop, RequestStop())
    HANDLE_EXPR(is_running, IsRunning())
    HANDLE_SUPERCLASS(FlowQueueable)
    HANDLE_SUPERCLASS(ObjectDir)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(Flow::DynamicPropertyEntry)
    SYNC_PROP_SET(name, o.mName, o.SetName(_val))
    SYNC_PROP_MODIFY(type, (int &)o.mType, o.ResetDefaultValues())
    SYNC_PROP_SET(default_int, o.mDefaultVal.Int(), o.mDefaultVal = _val)
    SYNC_PROP_SET(default_bool, o.mDefaultVal.Int(), o.mDefaultVal = _val)
    SYNC_PROP_SET(default_color, o.mDefaultVal.Int(), o.mDefaultVal = _val)
    SYNC_PROP_SET(default_float, o.mDefaultVal.Float(), o.mDefaultVal = _val)
    SYNC_PROP_SET(
        default_string,
        o.mDefaultVal.Type() == kDataString ? o.mDefaultVal.Str() : "",
        o.mDefaultVal = _val
    )
    SYNC_PROP_SET(
        default_object,
        o.mDefaultVal.Type() == kDataObject ? o.mDefaultVal.GetObj() : NULL_OBJ,
        o.mDefaultVal = _val
    )
    SYNC_PROP_SET(default_symbol, o.GetDefaultValueSymbol(), o.mDefaultVal = _val)
    SYNC_PROP_SET(object_class, o.mObjectClass, o.SetClassFilter(_val))
    SYNC_PROP(object_type, o.mObjectType)
    SYNC_PROP(help, o.mHelp)
    SYNC_PROP(exposed, o.mExposed)
    SYNC_PROP_SET(get_symbol_list, o.GetSymbolList(), )
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(Flow)
    SYNC_PROP(dynamic_properties, mDynamicProperties)
    SYNC_PROP_SET(start_on_enter, unk170 != 0, if (_val.Int() != 0) unk170 = 2;
                  else unk170 = 0)
    SYNC_PROP_SET(start_after_game_code, unk170 == 2, unk170 = _val.Int() != 0 ? 1 : 0)
    SYNC_PROP(hard_stop, mHardStop)
    SYNC_PROP(intensity, FlowNode::sIntensity)
    SYNC_PROP(private, mPrivate)
    SYNC_PROP_SET(toggle_running, 0, ToggleRunning(_val.Int()))
    SYNC_SUPERCLASS(FlowQueueable)
    SYNC_SUPERCLASS(ObjectDir)
END_PROPSYNCS
