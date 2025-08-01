#include "flow/FlowPtr.h"
#include "obj/Dir.h"

bool FlowPtrBase::RefreshParamObject(void) { return false; }

int FlowPtrBase::GetInitialState(Hmx::Object *obj) {
    if (obj == nullptr)
        return -3;
    if (ObjectDir::StaticClassName() == Symbol("EditDir"))
        return 5;
    else
        return 2;
}

Hmx::Object *FlowPtrBase::GetObject(void) { return nullptr; }

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *) { return nullptr; }

Hmx::Object *FlowPtrBase::LoadObject(BinStream &) { return nullptr; }
