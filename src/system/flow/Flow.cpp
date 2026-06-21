#include "flow/Flow.h"
#include "flow/FlowAnimate.h"
#include "flow/FlowCommand.h"
#include "flow/FlowDistance.h"
#include "flow/FlowEventListener.h"
#include "flow/FlowIf.h"
#include "flow/FlowLabel.h"
#include "flow/FlowManager.h"
#include "flow/FlowMultiSetProperty.h"
#include "flow/FlowNode.h"
#include "flow/FlowOnStop.h"
#include "flow/FlowOutPort.h"
#include "flow/FlowPickOne.h"
#include "flow/FlowQueueable.h"
#include "flow/FlowRun.h"
#include "flow/FlowSequence.h"
#include "flow/FlowSetProperty.h"
#include "flow/FlowSlider.h"
#include "flow/FlowSound.h"
#include "flow/FlowSwitch.h"
#include "flow/FlowSwitchCase.h"
#include "flow/FlowTimer.h"
#include "flow/FlowTrigger.h"
#include "flow/FlowValueCase.h"
#include "flow/FlowWhile.h"
#include "flow/PropertyEventProvider.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

bool Flow::sReflectingProperty;

#pragma region DynamicPropertyEntry

Flow::DynamicPropertyEntry::DynamicPropertyEntry(Hmx::Object *obj)
    : mName(""), mType(kInt), mHelp(""), mExposed(false) {
    mObjectClass = "Object";
    mObjectType = Symbol();
}

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

BinStream &operator<<(BinStream &bs, const Flow::DynamicPropertyEntry &entry);

void Flow::DynamicPropertyEntry::SetClassFilter(DataNode &filter) {
    if (filter.Sym() != mObjectClass) {
        mObjectClass = filter.Sym();
        mObjectType = Symbol();
    }
}

void Flow::DynamicPropertyEntry::SetName(DataNode &name) {
    String str(name.Str());
    str.ReplaceAll(' ', '_');
    str.ToLower();
    if (str == "name" || str == "note" || str == "intensity" || str == "hard_stop"
        || str == "private" || str == "interrupt") {
        str.insert(0, "_");
    }
    mName = str;
}

void Flow::DynamicPropertyEntry::ResetDefaultValues() { mDefaultVal = 0; }

Symbol Flow::DynamicPropertyEntry::GetDefaultValueSymbol() {
    if (mDefaultVal.Type() == kDataSymbol) {
        return mDefaultVal.Sym();
    } else {
        return Symbol();
    }
}

DataNode Flow::DynamicPropertyEntry::GetSymbolList() {
    if (unk24.Type() == kDataArray) {
        return unk24.Array();
    } else {
        DataArrayPtr ptr(new DataArray(1));
        ptr->Node(0) = Symbol();
        return ptr;
    }
}

#pragma endregion
#pragma region Flow

Flow::Flow()
    : mDynamicProperties(this), mFlowLabels(this), mFlowOutPorts(this), mObjects(this),
      unk170(0), mPrivate(1), mHardStop(0), unk178(0) {}

Flow::~Flow() {
    if (!mRunningNodes.empty()) {
        Flow *f = this;
        if (f->ProxyFile().empty()) {
            FlowQueueable::Deactivate(true);
        }
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

BEGIN_PROPSYNCS(Flow)
    SYNC_PROP(dynamic_properties, mDynamicProperties)
    SYNC_PROP_SET(start_on_enter, unk170 != 0, StartOnEnter(_val.Int()))
    SYNC_PROP_SET(start_after_game_code, unk170 == 2, StartAfterGameCode(_val.Int()))
    SYNC_PROP(hard_stop, mHardStop)
    SYNC_PROP(intensity, FlowNode::sIntensity)
    SYNC_PROP(private, mPrivate)
    SYNC_PROP_SET(toggle_running, 0, ToggleRunning(_val.Int()))
    SYNC_SUPERCLASS(FlowQueueable)
    SYNC_SUPERCLASS(ObjectDir)
END_PROPSYNCS

BEGIN_SAVES(Flow)
    SAVE_REVS(7, 2)
    DataArrayPtr ptr;
    FOREACH (it, mDynamicProperties) {
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
        FOREACH (it, mDynamicProperties) {
            bs << Symbol(it->mName.c_str());
            const DataNode *prop = Property(it->mName.c_str(), false);
            if (prop) {
                if (prop->Type() == kDataObject) {
                    bs << *prop;
                } else {
                    bs << prop->Type();
                    bs << *prop;
                }
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

BEGIN_COPYS(Flow)
    COPY_SUPERCLASS(ObjectDir)
    COPY_SUPERCLASS(FlowQueueable)
    CREATE_COPY(Flow)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDynamicProperties)
        FOREACH (it, mDynamicProperties) {
            Symbol prop(it->mName.c_str());
            SetProperty(prop, *c->Property(prop, false));
        }
        COPY_MEMBER(unk170)
        while (!mChildNodes.empty()) {
            delete mChildNodes[0];
        }
        FOREACH (it, c->mChildNodes) {
            FlowNode *n;
            Flow *cur = dynamic_cast<Flow *>((FlowNode *)*it);
            if (cur) {
                n = FlowNode::DuplicateChild(*it);
            } else {
                Hmx::Object *newObj = Hmx::Object::NewObject((*it)->ClassName());
                newObj->InitObject();
                n = dynamic_cast<FlowNode *>(newObj);
                n->SetParent(this, true);
                n->Copy(*it, kCopyDeep);
            }
            n->SetParent(this, true);
            n->MoveIntoDir(this, (ObjectDir *)c);
        }
        COPY_MEMBER(mPrivate)
        COPY_MEMBER(mHardStop)
        RefreshPortLabelLists();
        Flow *f = this;
        if (!f->ProxyFile().empty()) {
            mInterrupt = (QueueState)5;
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(Flow)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void Flow::PreSave(BinStream &bs) {
    Flow *f = this;
    SetInlineProxyType(
        !f->ProxyFile().empty() && IsProxy() ? kInlineCached : kInlineAlways
    );
}

INIT_REVS(7, 2)

void Flow::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(7, 2)
    ObjectDir::PreLoad(bs);
    if (d.altRev < 2 && !gLoadingProxyFromDisk) {
        if (strstr(ObjectDir::ProxyFile().c_str(), "/flow/")
            && !strstr(ObjectDir::ProxyFile().c_str(), "/run/flow/")) {
            FilePath &lmao = (FilePath &)ObjectDir::ProxyFile();
            lmao.Set(
                FileRoot(),
                MakeString("flow/%s", FileGetName(ObjectDir::ProxyFile().c_str()))
            );
        }
    }
    d.PushRev(this);
}

void Flow::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    ObjectDir::PostLoad(bs);
    if (IsProxy()) {
        int unkf0 = 0;
        bs >> unkf0;
    }
}

void Flow::SyncObjects() {
    ObjectDir::SyncObjects();

    FlowNode *n = this;
    Flow *topFlow;
    while ((topFlow = n->GetTopFlow()) && topFlow->GetParent()) {
        n = topFlow->GetParent();
    }
    ObjectDir *loadingDir = topFlow->Dir();
    if (dynamic_cast<Flow *>(loadingDir)) {
        DirLoader *dl = topFlow->Loader();
        loadingDir = dl ? dl->ProxyDir() : topFlow->Dir();
    }
    if (loadingDir && loadingDir != this) {
        FOREACH (it, mDynamicProperties) {
            if (it->mExposed) {
                Symbol name(it->mName.c_str());
                DataArrayPtr ptr(new DataArray(1));
                ptr->Node(0) = name;
                if (topFlow->HasPropertySink(this, ptr)) {
                    topFlow->RemovePropertySink(this, ptr);
                }
                if (HasPropertySink(this, ptr)) {
                    RemovePropertySink(this, ptr);
                }
                if (!topFlow->Property(ptr, false)) {
                    if (Property(ptr, false)) {
                        topFlow->SetProperty(ptr, *Property(ptr, true));
                    } else {
                        topFlow->SetProperty(ptr, it->mDefaultVal);
                    }
                } else {
                    SetProperty(ptr, *topFlow->Property(ptr, true));
                }
                topFlow->AddPropertySink(this, ptr, "on_reflected_property_changed");
                AddPropertySink(this, ptr, "on_internal_property_changed");
            }
        }
    }
}

bool Flow::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    if (ProxyFile().empty()) {
        Timer timer;
        timer.Restart();
        PushDrivenProperties();
        bool activate = FlowQueueable::Activate(nullptr);
        timer.Stop();
        TheFlowMgr->AddEventTime(Name(), timer.Ms());
        return activate;
    } else {
        PushDrivenProperties();
        return ActivateTrigger();
    }
}

void Flow::Deactivate(bool b1) { FlowQueueable::Deactivate(b1); }

void Flow::Enter() {
    Flow *f = this; // this actually affects codegen. :)
    if (f->ProxyFile().empty() && unk170 != 0) {
        if (unk170 == 1) {
            Execute(kQueue);
        } else {
            TheFlowMgr->QueueCommand(this, kQueue);
        }
    }
}

void Flow::Exit() {
    Flow *f = this;
    if (IsRunning() && f->ProxyFile().empty()) {
        if (mHardStop) {
            Deactivate(false);
        } else {
            RequestStop();
        }
    }
}

void Flow::RequestStop() {
    FLOW_LOG("RequestStop\n");
    if (!unk58) {
        unk58 = true;
        FOREACH (it, mRunningNodes) {
            if ((*it)->ClassName() != FlowLabel::StaticClassName()) {
                (*it)->RequestStop();
            }
        }
        unk58 = false;
    }
}

void Flow::RequestStopCancel() {
    if (unk58) {
        unk58 = false;
        FOREACH (it, mRunningNodes) {
            if ((*it)->ClassName() != FlowLabel::StaticClassName()) {
                (*it)->RequestStopCancel();
            }
        }
    }
}

void Flow::Execute(FlowNode::QueueState q) {
    FLOW_LOG("Start on Enter\n");
    if (IsRunning()) {
        Deactivate(false);
    }
    Activate();
}

Flow *Flow::GetOwnerFlow() {
    const FilePath &file = ProxyFile();
    return !file.empty() ? dynamic_cast<Flow *>(Dir()) : nullptr;
}

bool Flow::ActivateTrigger() {
    unk58 = false;
    FOREACH (it, mChildNodes) {
        if ((*it)->ClassName() != FlowLabel::StaticClassName()) {
            ActivateChild(*it);
        }
        if (unk58)
            break;
    }
    return FlowNode::IsRunning();
}

bool Flow::Activate(Hmx::Object *obj) {
    FLOW_LOG("activate with listener");
    unk58 = false;
    Timer timer;
    timer.Restart();
    PushDrivenProperties();
    bool activate = FlowQueueable::Activate(obj);
    timer.Stop();
    TheFlowMgr->AddEventTime(Name(), timer.Ms());
    return activate;
}

bool Flow::Activate(Hmx::Object *obj, DataArray *) { return Activate(obj); }

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

FlowLabel *Flow::GetLabelForSym(Symbol sym) {
    FOREACH (it, mFlowLabels) {
        FlowLabel *cur = *it;
        if (cur->Label() == sym) {
            return cur;
        }
    }
    return nullptr;
}

void ScanForOutPorts(ObjPtrVec<FlowOutPort> &, FlowNode *, Flow *);

void Flow::RefreshPortLabelLists() {
    mFlowOutPorts.clear();
    mFlowLabels.clear();
    ScanForOutPorts(mFlowOutPorts, this, this);
    FOREACH (it, mChildNodes) {
        if ((*it)->ClassName() == "FlowLabel") {
            FlowNode *cur = *it;
            mFlowLabels.push_back(static_cast<FlowLabel *>(cur));
        }
    }
}

void Flow::OnReflectedPropertyChanged(DataArray *a) {
    if (!sReflectingProperty) {
        Symbol sym = a->Array(2)->Sym(0);
        DataArray *propArr = a->Array(2);
        Flow *topFlow = GetTopFlow();
        ObjectDir *flowDir = topFlow->Dir();
        FOREACH (it, mDynamicProperties) {
            if (it->mExposed && sym == it->mName.c_str()) {
                sReflectingProperty = true;
                SetProperty(propArr, *flowDir->Property(propArr, true));
                sReflectingProperty = false;
            }
        }
    }
}

void Flow::OnInternalPropertyChanged(DataArray *a) {
    if (!sReflectingProperty) {
        Symbol sym = a->Array(2)->Sym(0);
        DataArray *propArr = a->Array(2);
        Flow *topFlow = GetTopFlow();
        ObjectDir *flowDir = topFlow->Dir();
        FOREACH (it, mDynamicProperties) {
            if (it->mExposed && sym == it->mName.c_str()) {
                sReflectingProperty = true;
                flowDir->SetProperty(propArr, *Property(propArr, true));
                sReflectingProperty = false;
            }
        }
    }
}

void Flow::ApplyParams(DataArray *msg, FlowTrigger *trigger) {
    unk178++;
    if (msg && msg->Size() >= 3) {
        DataArray *def = trigger->GetEventEditorDef(MsgSinks::CurrentExportEvent());
        if (def) {
            def = def->FindArray("editor");
            for (int i = 1; i < def->Size(); i++) {
                MILO_ASSERT(def->Type(i) == kDataArray, 0x24F);
                Symbol prop = def->Array(i)->Sym(0);
                MILO_ASSERT(msg->Size() > i + 1, 0x251);
                SetProperty(prop, msg->Evaluate(i + 1));
            }
        }
    }
}

void FlowInit() {
    TheFlowMgr = new FlowManager();
    REGISTER_OBJ_FACTORY(FlowNode);
    REGISTER_OBJ_FACTORY(FlowEventListener);
    REGISTER_OBJ_FACTORY(FlowSequence);
    REGISTER_OBJ_FACTORY(FlowAnimate);
    REGISTER_OBJ_FACTORY(FlowPickOne);
    REGISTER_OBJ_FACTORY(Flow);
    REGISTER_OBJ_FACTORY(FlowSwitch);
    REGISTER_OBJ_FACTORY(FlowSwitchCase);
    REGISTER_OBJ_FACTORY(FlowSound);
    REGISTER_OBJ_FACTORY(FlowRun);
    REGISTER_OBJ_FACTORY(FlowTimer);
    REGISTER_OBJ_FACTORY(FlowValueCase);
    REGISTER_OBJ_FACTORY(FlowOutPort);
    REGISTER_OBJ_FACTORY(FlowSetProperty);
    REGISTER_OBJ_FACTORY(FlowMultiSetProperty);
    REGISTER_OBJ_FACTORY(FlowWhile);
    REGISTER_OBJ_FACTORY(FlowLabel);
    REGISTER_OBJ_FACTORY(FlowCommand);
    REGISTER_OBJ_FACTORY(FlowIf);
    REGISTER_OBJ_FACTORY(FlowOnStop);
    REGISTER_OBJ_FACTORY(FlowSlider);
    REGISTER_OBJ_FACTORY(FlowDistance);
    REGISTER_OBJ_FACTORY(PropertyEventProvider);
    if (streq(FileRoot(), FileSystemRoot())) {
        PropertyEventProvider *prov = Hmx::Object::New<PropertyEventProvider>();
        prov->SetType("example");
        prov->SetName("exampleData", ObjectDir::Main());
    }
}

#pragma endregion
