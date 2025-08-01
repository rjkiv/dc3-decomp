#include "flow/FlowPtr.h"
#include "obj/Dir.h"

bool FlowPtrBase::RefreshParamObject(void) { return false; }

int FlowPtrBase::GetInitialState(Hmx::Object *obj) {
    if (obj == nullptr)
        return -3;
    u8 b = ObjectDir::StaticClassName() == "EditDir";
    b = !b;
    return b;
}

Hmx::Object *FlowPtrBase::GetObject(void) { return nullptr; }

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *) { return nullptr; }

Hmx::Object *FlowPtrBase::LoadObject(BinStream &) { return nullptr; }
