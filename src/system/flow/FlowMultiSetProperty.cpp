#include "flow/FlowMultiSetProperty.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"

FlowMultiSetProperty::FlowMultiSetProperty()
    : mTargetObjects(this, (EraseMode)1, kObjListNoNull) {}

FlowMultiSetProperty::~FlowMultiSetProperty() {}

BEGIN_HANDLERS(FlowMultiSetProperty)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowMultiSetProperty)
    SYNC_PROP_MODIFY(targets, mTargetObjects, {
        mTargetObjects.sort(ObjNameSort());
        mTargetObjects.unique();
    })
    SYNC_PROP(value, mValue)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowMultiSetProperty)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mTargetObjects;
    bs << mProperty << mValue;
END_SAVES

BEGIN_COPYS(FlowMultiSetProperty)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowMultiSetProperty, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mTargetObjects)
        COPY_MEMBER(mProperty)
        COPY_MEMBER(mValue)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowMultiSetProperty)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    mTargetObjects.Load(d, true, static_cast<Flow *>(Dir())->LoadingDir());
    d >> mProperty >> mValue;
END_LOADS

bool FlowMultiSetProperty::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (!mTargetObjects.empty()) {
        if (GetDrivenEntry("value")) {
            Hmx::Object *o = mTargetObjects[0];
            mValue = o->Property(mProperty.Array())->Evaluate();
        }
    }
    FlowNode::PushDrivenProperties();
    FOREACH (it, mTargetObjects) {
        if (*it) {
            (*it)->SetProperty(mProperty.Array(), mValue);
        }
    }
    return false;
}
