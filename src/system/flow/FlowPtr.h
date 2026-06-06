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
        : mObjName(name), mOwnerNode(node), mState(-3) {}

    int GetInitialState(Hmx::Object *);

    FlowPtrBase &operator=(const FlowPtrBase &other) {
        mObjName = other.mObjName;
        mState = other.mState;
        return *this;
    }

    void Reset() {
        mObjName = 0;
        mState = GetInitialState(nullptr);
    }

protected:
    bool RefreshParamObject();
    Hmx::Object *GetObject();
    Hmx::Object *LoadObject(BinStream &);

    Symbol mObjName; // 0x0
    FlowNode *mOwnerNode; // 0x4
    int mState; // 0x8
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
          mObjPtr(owner, ptr) {}
    FlowPtr(const FlowPtr &);
    ~FlowPtr() {}

    // see: merged_82401EF0
    void operator=(T *obj) {
        int state = GetInitialState(obj);
        mObjName = !obj ? 0 : obj->Name();
        mObjPtr = obj;
        mState = state;
    }

    FlowPtr &operator=(const FlowPtr &ptr) {
        auto state = ptr.mState;
        auto objname = ptr.mObjName;
        mObjPtr = ptr.mObjPtr.Ptr();
        mObjName = objname;
        mState = state;
        return *this;
    }

    void Reset() {
        mObjPtr = nullptr;
        FlowPtrBase::Reset();
    }

    operator T *() { return Get(); }

    T *Ptr() const { return mObjPtr; }

    T *operator->() {
        T *o = Get();
        MILO_ASSERT(o, 0xB2);
        return o;
    }

    T *LoadFromMainOrDir(BinStream &bs) {
        mObjPtr = dynamic_cast<T *>(LoadObject(bs));
        return mObjPtr;
    }

    void Save(BinStream &bs) const {
        if (mObjPtr && mState == -2) {
            bs << mObjPtr;
        } else {
            bs << mObjName;
        }
    }

private:
    T *Get() {
        if (mState >= -1 && RefreshParamObject()) {
            mObjPtr = dynamic_cast<T *>(GetObject());
        }
        return mObjPtr;
    }

    ObjPtr<T> mObjPtr; // 0xc
};

template <typename T>
BinStream &operator<<(BinStream &bs, const FlowPtr<T> &ptr) {
    ptr.Save(bs);
    return bs;
}

template <typename T>
BinStream &operator>>(BinStream &, FlowPtr<T> &);

template <class T>
bool PropSync(FlowPtr<T> &ptr, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropSet) {
        if (node.Type() == kDataObject && node.GetObj()) {
            ptr.mObjName = node.GetObj()->Name();
            ptr.mState = ptr.GetInitialState(node.GetObj());
        } else
            ptr.mObjName = 0;
    }
    return PropSync(ptr.mObjPtr, node, prop, i, op);
}
