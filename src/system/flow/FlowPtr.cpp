#include "flow/FlowPtr.h"
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"

bool FlowPtrBase::RefreshParamObject() {
    Flow *owner = mOwnerNode->GetOwnerFlow();
    if (owner && mState != owner->GetNumParams()) {
        mState = owner->GetNumParams();
        return true;
    } else
        return false;
}

int FlowPtrBase::GetInitialState(Hmx::Object *obj) {
    if (!obj)
        return -3;
    else
        return ObjectDir::StaticClassName() == "EditDir" ? -1 : -2;
}

Hmx::Object *FlowPtrBase::GetObject() {
    if (!mObjName.Null()) {
        ObjectDir *owner = mOwnerNode->GetOwnerFlow();
        if (owner) {
            const DataNode *prop = owner->Property(mObjName, false);
            if (prop && prop->Type() == kDataObject) {
                Hmx::Object *obj = prop->GetObj();
                if (obj)
                    return obj;
            }
        }
        ObjectDir *owner2 = mOwnerNode->GetOwnerFlow();
        if (owner2) {
            ObjectDir *dir = owner2->Dir();
            if (dir) {
                Hmx::Object *obj = dir->FindObject(mObjName.Str(), false, true);
                if (obj) {
                    mState = -2;
                    return obj;
                }
            }
        }
        mState = -1;
    }
    return nullptr;
}

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *dir) {
    Flow *flow = dynamic_cast<Flow *>(dir);
    if (flow) {
        return flow->Loader() ? flow->Loader()->ProxyDir() : flow->Dir();
    } else
        return nullptr;
}

Hmx::Object *FlowPtrBase::LoadObject(BinStream &bs) {
    bs >> mObjName;
    return nullptr;
}
