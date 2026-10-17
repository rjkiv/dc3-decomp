#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

class FlowPtrBase {
public:
    FlowPtrBase(Symbol name, FlowNode *node)
        : mSymbol(name), mOwner(node), mParamIdx(-3) {}

    int GetInitialState(Hmx::Object *);

    FlowPtrBase &operator=(const FlowPtrBase &other) {
        mSymbol = other.mSymbol;
        mParamIdx = other.mParamIdx;
        return *this;
    }

    void Reset() {
        mSymbol = 0;
        mParamIdx = GetInitialState(nullptr);
    }

    Symbol mSymbol; // 0x0
    FlowNode *mOwner; // 0x4
    int mParamIdx; // 0x8

protected:
    bool RefreshParamObject();
    Hmx::Object *GetObject();
    Hmx::Object *LoadObject(BinStream &);
};

ObjectDir *FlowPtrGetLoadingDir(ObjectDir *);

template <class T>
class FlowPtr : public FlowPtrBase {
    template <class U>
    friend bool
    PropSync(FlowPtr<U> &ptr, DataNode &node, DataArray *prop, int i, PropOp op);

public:
    FlowPtr(Hmx::Object *owner, T *ptr = nullptr)
        : FlowPtrBase(ptr ? ptr->Name() : 0, dynamic_cast<FlowNode *>(owner)),
          mObject(owner, ptr) {}
    ~FlowPtr() {}

    // see: merged_82401EF0
    void operator=(T *obj) {
        int state = GetInitialState(obj);
        mSymbol = !obj ? 0 : obj->Name();
        mObject = obj;
        mParamIdx = state;
    }

    FlowPtr &operator=(const FlowPtr &ptr) {
        auto state = ptr.mParamIdx;
        auto objname = ptr.mSymbol;
        mObject = ptr.mObject.Ptr();
        mSymbol = objname;
        mParamIdx = state;
        return *this;
    }

    void Reset() {
        mObject = nullptr;
        FlowPtrBase::Reset();
    }

    operator T *() { return Get(); }

    T *Ptr() const { return mObject; }

    T *operator->() {
        T *o = Get();
        MILO_ASSERT(o, 0xB2);
        return o;
    }

    T *LoadFromMainOrDir(BinStream &bs) {
        mObject = dynamic_cast<T *>(LoadObject(bs));
        return mObject;
    }

    void Save(BinStream &bs) const {
        if (mObject && mParamIdx == -2) {
            bs << mObject;
        } else {
            bs << mSymbol;
        }
    }

    void Load(BinStream &bs) { mObject = LoadObject(bs); }

    ObjPtr<T> mObject; // 0xc

private:
    T *Get() {
        if (mParamIdx >= -1 && RefreshParamObject()) {
            mObject = dynamic_cast<T *>(GetObject());
        }
        return mObject;
    }
};

template <typename T>
BinStream &operator<<(BinStream &bs, const FlowPtr<T> &ptr) {
    ptr.Save(bs);
    return bs;
}

template <typename T>
BinStream &operator>>(BinStream &bs, FlowPtr<T> &ptr) {
    ptr.Load(bs);
    return bs;
}

template <class T>
bool PropSync(FlowPtr<T> &ptr, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropSet) {
        if (node.Type() == kDataObject && node.GetObj()) {
            ptr.mSymbol = node.GetObj()->Name();
            ptr.mParamIdx = ptr.GetInitialState(node.GetObj());
        } else
            ptr.mSymbol = 0;
    }
    return PropSync(ptr.mObject, node, prop, i, op);
}
