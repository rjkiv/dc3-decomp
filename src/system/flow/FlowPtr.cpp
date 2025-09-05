#include "flow/FlowPtr.h"
#include "flow/FlowNode.h"
#include "obj/Dir.h"
#include "obj/Object.h"

bool FlowPtrBase::RefreshParamObject() {
    Flow *owner = mOwnerNode->GetOwnerFlow();
    if (owner) {
    }
    return false;
}

int FlowPtrBase::GetInitialState(Hmx::Object *obj) {
    if (!obj)
        return -3;
    else
        return ObjectDir::StaticClassName() == "EditDir" ? -1 : -2;
}

Hmx::Object *FlowPtrBase::GetObject(void) { return nullptr; }

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *) { return nullptr; }

Hmx::Object *FlowPtrBase::LoadObject(BinStream &bs) {
    bs >> mObjName;
    return nullptr;
}
