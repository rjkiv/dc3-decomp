#include "flow/FlowPtr.h"
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"

bool FlowPtrBase::RefreshParamObject() {
    Flow *owner = mOwner->GetOwnerFlow();
    if (owner && mParamIdx != owner->GetParamIdx()) {
        mParamIdx = owner->GetParamIdx();
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
    if (!mSymbol.Null()) {
        ObjectDir *owner = mOwner->GetOwnerFlow();
        if (owner) {
            const DataNode *prop = owner->Property(mSymbol, false);
            if (prop && prop->Type() == kDataObject) {
                Hmx::Object *obj = prop->GetObj();
                if (obj)
                    return obj;
            }
        }
        ObjectDir *owner2 = mOwner->GetOwnerFlow();
        if (owner2) {
            ObjectDir *dir = owner2->Dir();
            if (dir) {
                Hmx::Object *obj = dir->FindObject(mSymbol.Str(), false, true);
                if (obj) {
                    mParamIdx = -2;
                    return obj;
                }
            }
        }
        mParamIdx = -1;
    }
    return nullptr;
}

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *dir) {
    Flow *flow = dynamic_cast<Flow *>(dir);
    if (flow) {
        return flow->LoadingDir();
    } else
        return nullptr;
}

Hmx::Object *FlowPtrBase::LoadObject(BinStream &bs) {
    bs >> mSymbol;
    ObjectDir *loadingDir = mOwner->Dir();
    Hmx::Object *obj = loadingDir->FindObject(mSymbol.Str(), false, true);
    if (obj) {
        mParamIdx = -2;
        return obj;
    } else {
        loadingDir = FlowPtrGetLoadingDir(loadingDir);
        if (loadingDir) {
            obj = loadingDir->FindObject(mSymbol.Str(), false, true);
            if (obj) {
                mParamIdx = -2;
                return obj;
            } else {
                loadingDir = FlowPtrGetLoadingDir(loadingDir);
                if (loadingDir) {
                    obj = loadingDir->FindObject(mSymbol.Str(), false, true);
                    if (obj) {
                        mParamIdx = -2;
                        return obj;
                    } else {
                        obj = ObjectDir::Main()->FindObject(mSymbol.Str(), false, true);
                        if (obj) {
                            mParamIdx = -2;
                            return obj;
                        } else {
                            if (!mSymbol.Null()) {
                                mParamIdx = -1;
                                return obj;
                            } else
                                return nullptr;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}
