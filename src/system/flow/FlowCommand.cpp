#include "flow/FlowCommand.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "flow/Flow.h"
#include "obj/Utl.h"

FlowCommand::FlowCommand() : mObject(this), mHandler(0) {}
FlowCommand::~FlowCommand() {}

BEGIN_HANDLERS(FlowCommand)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowCommand)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowCommand)
    SAVE_REVS(3, 0)
    int size = mTypeProps ? mTypeProps->Map()->Size() : 0;
    DataArrayPtr ptr(new DataArray(size));
    bs << size;
    for (int i = 0; i < size; i++) {
        bs << mTypeProps->Map()->Evaluate(i);
        ptr->Node(i) = mTypeProps->Map()->Evaluate(i);
    }
    ClearAllTypeProps();
    SAVE_SUPERCLASS(FlowNode)
    for (int i = 0; i < size; i += 2) {
        SetProperty(ptr->Sym(i), ptr->Node(i + 1));
    }
    bs << mObject;
    bs << mHandler;
END_SAVES

BEGIN_COPYS(FlowCommand)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowCommand)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mObject)
        COPY_MEMBER(mHandler)
        PushDrivenProperties();
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(FlowCommand)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)

    std::list<Symbol> symbols;
    std::list<DataNode> datanodes;
    if (d.rev > 2) {
        int count;
        bs >> count;
        Flow *owner = GetOwnerFlow();
        ObjectDir *dir = owner->Dir();
        for (int i = 0; i < count; i += 2) {
            DataNode n;
            n.Load(bs, dir);
            symbols.push_back(n.Sym());
            DataNode n2;
            n2.Load(bs, dir);
            datanodes.push_back(n2);
        }
    }
    FlowNode::Load(bs);
    ClearAllTypeProps();
    auto sit = symbols.begin();
    for (auto dit = datanodes.begin(); dit != datanodes.end(); ++sit, ++dit) {
        SetProperty(*sit, *dit);
    }
    if (d.rev < 1) {
        mObject = LoadObjectFromMainOrDir(bs, Dir());
    } else {
        bs >> mObject;
    }
    bs >> mHandler;
    if (d.rev < 2) {
        DataNode n;
        Flow *owner = GetOwnerFlow();
        ObjectDir *dir = owner->Dir();
        n.Load(bs, dir);
        if (n.Type() == kDataArray) {
            for (int i = 0; i < n.Array()->Size(); i++) {
                SetProperty(n.Array()->Array(i)->Sym(0), n.Array()->Array(i)->Node(1));
            }
        }
    } else if (d.rev < 3) {
        int count;
        bs >> count;
        Flow *owner = GetOwnerFlow();
        ObjectDir *dir = owner->Dir();
        for (int i = 0; i < count; i += 2) {
            DataNode n1;
            n1.Load(bs, dir);
            DataNode n2;
            n2.Load(bs, dir);
            SetProperty(n1.Sym(), n2.Evaluate());
        }
    }
    DataNode handlerDef = GetHandlerDef();
    if (handlerDef.Type() == kDataArray) {
        DataArray *a = handlerDef.Array();
        if (a && a->Size() > 2) {
            for (int i = 2; i < a->Size(); i++) {
                DataArray *prop = a->Array(i);
                if (!Property(prop->Sym(0), false)) {
                    SetProperty(prop->Sym(0), prop->Node(1));
                }
            }
        }
    }
END_LOADS

bool FlowCommand::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    PushDrivenProperties();
    if (mObject && !mHandler.Null()) {
        int size = mTypeProps ? mTypeProps->Size() : 0;
        Message msg(size);
        msg.SetType(mHandler);
        for (int i = 0; i < size; i += 2) {
            msg[i] = mTypeProps->Map()->Evaluate(i + 1);
        }
        mObject->Handle(msg, false);
    }
    return false;
}

DataNode FlowCommand::GetHandlerDef() {
    if (mObject && !mHandler.Null()) {
        DataArray *typeDef = mObject->TypeDef();
        if (typeDef) {
            if (typeDef->FindArray("flow_commands", false)) {
                DataArray *cmds = typeDef->FindArray("flow_commands");
                if (cmds->FindArray(mHandler, false)) {
                    return cmds->FindArray(mHandler);
                } else {
                    for (int i = 1; i < cmds->Size(); i++) {
                        if (cmds->Type(i) == kDataArray) {
                            if (cmds->Array(i)->Sym(0) == mHandler) {
                                return cmds->Array(i);
                            }
                        }
                    }
                }
            }
        }
        std::vector<Symbol> superClasses;
        superClasses.push_back(mObject->ClassName());
        ListSuperClasses(mObject->ClassName(), superClasses);
        for (int i = 0; i < superClasses.size(); i++) {
            DataArray *cmds =
                mObject->ObjectDef(superClasses[i])->FindArray("flow_commands", false);
            if (cmds) {
                DataArray *ret = cmds->FindArray(mHandler, false);
                if (ret) {
                    return ret;
                }
                for (int i = 1; i < cmds->Size(); i++) {
                    if (cmds->Type(i) == kDataArray) {
                        if (cmds->Array(i)->Sym(0) == mHandler) {
                            return cmds->Array(i);
                        }
                    }
                }
            }
        }
        return 0;
    } else {
        return 0;
    }
}
