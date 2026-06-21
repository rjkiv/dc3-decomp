#include "flow/FlowMultiSetProperty.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"

FlowMultiSetProperty::FlowMultiSetProperty()
    : unk5c(this, (EraseMode)1, kObjListNoNull) {}

FlowMultiSetProperty::~FlowMultiSetProperty() {}

BEGIN_HANDLERS(FlowMultiSetProperty)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowMultiSetProperty)
    SYNC_PROP_MODIFY(targets, unk5c, {
        unk5c.sort(ObjNameSort());
        unk5c.unique();
    })
    SYNC_PROP(value, unk78)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowMultiSetProperty)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << unk5c;
    bs << unk70 << unk78;
END_SAVES

BEGIN_COPYS(FlowMultiSetProperty)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowMultiSetProperty, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(unk5c)
        COPY_MEMBER(unk70)
        COPY_MEMBER(unk78)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowMultiSetProperty)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    DirLoader *dl = Dir()->Loader();
    ObjectDir *dir = dl ? dl->ProxyDir() : Dir()->Dir();
    unk5c.Load(d.stream, true, dir);
    d >> unk70 >> unk78;
END_LOADS

bool FlowMultiSetProperty::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (!unk5c.empty()) {
        if (GetDrivenEntry("value")) {
            unk78 = unk5c[0]->Property(unk70.Array())->Evaluate();
        }
    }
    FlowNode::PushDrivenProperties();
    FOREACH (it, unk5c) {
        if (*it) {
            (*it)->SetProperty(unk70.Array(), unk78);
        }
    }
    return false;
}
