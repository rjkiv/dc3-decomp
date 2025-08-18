#pragma once
// #include "obj/Dir.h" /* IWYU pragma: keep */
#include "utl/BinStream.h" /* IWYU pragma: keep */
#include "os/Debug.h" /* IWYU pragma: keep */
#include <cstddef> /* IWYU pragma: keep */

// DO NOT try to include this header directly!
// include obj/Object.h instead

// ------------------------------------------------
// ObjRefConcrete
// ------------------------------------------------

template <class T1, class T2>
ObjRefConcrete<T1, T2>::ObjRefConcrete(T1 *obj) : mObject(obj) {
    if (mObject)
        mObject->AddRef(this);
}

template <class T1, class T2>
ObjRefConcrete<T1, T2>::ObjRefConcrete(const ObjRefConcrete &o) : mObject(o.mObject) {
    if (mObject)
        mObject->AddRef(this);
}

template <class T1, class T2>
ObjRefConcrete<T1, T2>::~ObjRefConcrete() {
    if (mObject)
        mObject->Release(this);
}

template <class T1, class T2>
void ObjRefConcrete<T1, T2>::SetObjConcrete(T1 *obj) {
    if (mObject)
        mObject->Release(this);
    mObject = obj;
    if (mObject)
        mObject->AddRef(this);
}

template <class T1, class T2>
Hmx::Object *ObjRefConcrete<T1, T2>::SetObj(Hmx::Object *root_obj) {
    T1 *obj = root_obj ? dynamic_cast<T1 *>(root_obj) : nullptr;
    SetObjConcrete(obj);
    return mObject ? mObject : nullptr;
}

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjRefConcrete<T1, class ObjectDir> &f) {
    MILO_ASSERT(f.RefOwner(), 0x4D1);
    const char *objName = f ? f->Name() : "";
    bs << objName;
    return bs;
}

// ------------------------------------------------
// ObjPtr
// ------------------------------------------------

template <class T>
ObjPtr<T>::ObjPtr(Hmx::Object *owner, T *ptr = nullptr)
    : ObjRefConcrete<T>(ptr), mOwner(owner) {}

template <class T>
ObjPtr<T>::ObjPtr(const ObjPtr &p) : ObjRefConcrete(p), mOwner(p.mOwner) {}

template <class T>
ObjPtr<T>::~ObjPtr() {}

template <class T>
BinStream &operator>>(BinStream &bs, ObjPtr<T> &ptr) {
    ptr.Load(bs, true, nullptr);
    return bs;
}

// ------------------------------------------------
// ObjOwnerPtr
// ------------------------------------------------

template <class T>
ObjOwnerPtr<T>::ObjOwnerPtr(ObjRefOwner *owner, T *ptr)
    : ObjRefConcrete<T>(ptr), mOwner(owner) {
    MILO_ASSERT(owner, 0xC8);
}

template <class T>
ObjOwnerPtr<T>::ObjOwnerPtr(const ObjOwnerPtr &o)
    : ObjRefConcrete(o.mObject), mOwner(o.mOwner) {
    MILO_ASSERT(mOwner, 0xCE);
}

template <class T>
ObjOwnerPtr<T>::~ObjOwnerPtr() {}

template <class T>
Hmx::Object *ObjOwnerPtr<T>::RefOwner() const {
    return mObject->RefOwner();
}

// ------------------------------------------------
// ObjPtrVec
// ------------------------------------------------

template <class T1, class T2>
void ObjPtrVec<T1, T2>::Set(iterator it, T1 *obj) {
    if (!obj && mListMode == 0) {
        erase(it);
    } else
        it->SetObjConcrete(obj);
}

// see Draw.cpp for this
template <class T1, class T2>
void ObjPtrVec<T1, T2>::operator=(const ObjPtrVec &other) {
    if (this != &other) {
        mNodes.clear();
    }
    mNodes.reserve(other.mNodes.size());
    for (const_iterator it = other.begin(); it != other.end(); ++it) {
        mNodes.push_back(Node(this));
        Set(end(), *it);
    }
}

// ------------------------------------------------
// ObjPtrList
// ------------------------------------------------

template <class T1, class T2>
void ObjPtrList<T1, T2>::pop_back() {
    MILO_ASSERT(mNodes != NULL, 0x18B);
    erase(mNodes->prev);
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::push_back(T1 *obj) {
    insert(end(), obj);
}

template <class T1, class T2>
typename ObjPtrList<T1, T2>::iterator ObjPtrList<T1, T2>::find(const Hmx::Object *target
) const {
    for (iterator it = begin(); it != end(); ++it) {
        if (*it == target)
            return it;
    }
    return end();
}

template <class T1, class T2>
bool ObjPtrList<T1, T2>::Load(BinStream &bs, bool, ObjectDir *, bool) {
    clear();
    int count;
    bs >> count;
    return false;
}

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrList<T1, ObjectDir> &list) {
    list.Load(bs, true, nullptr, true);
    return bs;
}
