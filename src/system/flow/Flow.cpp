#include "flow/Flow.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"
#include "utl/BinStream.h"

Flow::Flow()
    : mDynamicProperties(this), mFlowLabels(this), mFlowOutPorts(this), mObjects(this),
      unk170(0), mPrivate(1), mHardStop(0), unk178(0) {}

Flow::~Flow() {
    if (!mRunningNodes.empty()) {
        FlowQueueable::Deactivate(true);
    }
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

BinStream &operator<<(BinStream &bs, const Flow::DynamicPropertyEntry &entry);

BEGIN_SAVES(Flow)
    SAVE_REVS(7, 2)
    DataArrayPtr ptr;
    for (ObjVector<DynamicPropertyEntry>::iterator it = mDynamicProperties.begin();
         it != mDynamicProperties.end();
         ++it) {
        Symbol name = it->mName.c_str();
        const DataNode *prop = Property(name, false);
        if (prop && prop->Type() == kDataObject) {
            DataArrayPtr ptr2(new DataArray(2));
            ptr2->Node(0) = name;
            ptr2->Node(1) = prop->Evaluate();
            ptr->Resize(ptr->Size() + 1);
            ptr->Node(ptr->Size() - 1) = ptr2;
            mTypeProps->ClearKeyValue(name);
        }
    }
    if (mTypeProps && mTypeProps->Map()) {
        DataArray *props = mTypeProps->Map();
        for (int i = 0; i < props->Size(); i += 2) {
            Symbol sym = props->Sym(i);
            if (props->Type(i + 1) == kDataObject) {
                DataArrayPtr ptr3(new DataArray(2));
                ptr3->Node(0) = sym;
                ptr3->Node(1) = props->Evaluate(i + 1);
                ptr->Resize(ptr->Size() + 1);
                ptr->Node(ptr->Size() - 1) = ptr3;
                mTypeProps->ClearKeyValue(sym);
            }
        }
    }
    SAVE_SUPERCLASS(ObjectDir)
    for (int i = 0; i < ptr->Size(); i++) {
        mTypeProps->SetKeyValue(ptr->Array(i)->Sym(0), ptr->Array(i)->Node(1), true);
    }
    if (IsProxy()) {
        bs << mDynamicProperties.size();
        for (ObjVector<DynamicPropertyEntry>::iterator it = mDynamicProperties.begin();
             it != mDynamicProperties.end();
             ++it) {
            bs << Symbol(it->mName.c_str());
            const DataNode *prop = Property(it->mName.c_str(), false);
            if (prop) {
                if (prop->Type() != kDataObject) {
                    bs << prop->Type();
                }
                bs << *prop;
            } else {
                bs << it->mDefaultVal.Type();
                bs << it->mDefaultVal;
            }
        }

    } else {
        SAVE_SUPERCLASS(FlowQueueable)
        bs << mHardStop;
        bs << mDynamicProperties;
        bs << unk170;
        bs << mPrivate;
    }
END_SAVES

Flow *Flow::GetOwnerFlow() {
    const FilePath &file = ProxyFile();
    return !file.empty() ? dynamic_cast<Flow *>(Dir()) : nullptr;
}

void Flow::Execute(FlowNode::QueueState q) {
    FLOW_LOG("Start on Enter\n");
    if (IsRunning()) {
        Deactivate(false);
    }
    Activate();
}

void Flow::ToggleRunning(int type) {
    switch (type) {
    case 0:
        if (!IsRunning()) {
            FlowQueueable::Activate(nullptr);
        }
        break;
    case 1:
        Deactivate(false);
        break;
    case 2:
        RequestStop();
        break;
    default:
        MILO_FAIL("Unknown toggle running type: %d\n", type);
        break;
    }
}
