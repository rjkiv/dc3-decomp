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

    void operator=(T *obj) {
        mObjName = obj ? obj->Name() : 0;
        mState = GetInitialState(obj);
        mObjPtr = obj;
    }

    FlowPtr &operator=(const FlowPtr &ptr) {
        mObjPtr = ptr.mObjPtr;
        FlowPtrBase::operator=(ptr);
        return *this;
    }

    void Reset() {
        mObjPtr = nullptr;
        FlowPtrBase::Reset();
    }

    operator T *() { return Get(); }

    T *operator->() {
        T *o = Get();
        MILO_ASSERT(o, 0xB2);
        return o;
    }

    T *LoadFromMainOrDir(BinStream &bs) {
        mObjPtr = dynamic_cast<T *>(LoadObject(bs));
        return mObjPtr;
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
BinStream &operator<<(BinStream &, const FlowPtr<T> &);

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
