#pragma once
#include "Object.h"
#include "obj/Dir.h" /* IWYU pragma: keep */
#include "obj/Object.h"
#include "utl/BinStream.h" /* IWYU pragma: keep */
#include "os/Debug.h" /* IWYU pragma: keep */
#include "utl/PoolAlloc.h"
#include <cstddef> /* IWYU pragma: keep */

// DO NOT try to include this header directly!
// include obj/Object.h instead

#pragma region ObjRefConcrete
// ------------------------------------------------
// ObjRefConcrete
// ------------------------------------------------

template <class T1, class T2>
ObjRefConcrete<T1, T2>::ObjRefConcrete(T1 *obj) : mObject(obj) {
    if (mObject)
        mObject->AddRef(this);
}

template <class T1, class T2>
__forceinline ObjRefConcrete<T1, T2>::ObjRefConcrete(const ObjRefConcrete &o)
    : mObject(o.mObject) {
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
    return mObject;
}

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjRefConcrete<T1, class ObjectDir> &f) {
    MILO_ASSERT(f.RefOwner(), 0x4D1);
    T1 *obj = f;
    const char *objName = obj ? obj->Name() : "";
    bs << objName;
    return bs;
}

template <class T1, class T2>
bool ObjRefConcrete<T1, T2>::Load(BinStream &bs, bool print, ObjectDir *dir) {
    char buf[128];
    bs.ReadString(buf, 128);
    Hmx::Object *refOwner = RefOwner();
    if (!dir && refOwner) {
        dir = refOwner->Dir();
    }
    if (refOwner && dir) {
        SetObj(dir->FindObject(buf, false, true));
        if (!mObject && buf[0] != '\0') {
            if (print) {
                MILO_NOTIFY(
                    "%s couldn't find %s in %s", PathName(refOwner), buf, PathName(dir)
                );
            }
            return false;
        }
    } else {
        if (mObject) {
            Unlink();
        }
        mObject = nullptr;
        if (buf[0] != '\0') {
            if (print)
                MILO_NOTIFY("No dir to find %s", buf);
        }
    }
    return true;
}

#pragma endregion
#pragma region ObjPtr
// ------------------------------------------------
// ObjPtr
// ------------------------------------------------

template <class T>
ObjPtr<T>::ObjPtr(Hmx::Object *owner, T *ptr = nullptr)
    : ObjRefConcrete<T>(ptr), mOwner(owner) {}

template <class T>
ObjPtr<T>::ObjPtr(const ObjPtr &p) : ObjRefConcrete(p), mOwner(p.mOwner) {}

template <class T>
BinStream &operator>>(BinStream &bs, ObjPtr<T> &ptr) {
    ptr.Load(bs, true, nullptr);
    return bs;
}

#pragma endregion
#pragma region ObjOwnerPtr
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

// template <class T1>
// BinStream &operator<<(BinStream &bs, const ObjOwnerPtr<T1> &ptr);

template <class T1>
BinStream &operator>>(BinStream &bs, ObjOwnerPtr<T1> &ptr) {
    ptr.Load(bs, true, nullptr);
    return bs;
}

#pragma endregion
#pragma region ObjPtrVec
// ------------------------------------------------
// ObjPtrVec
// ------------------------------------------------

template <class T1, class T2>
ObjPtrVec<T1, T2>::ObjPtrVec(Hmx::Object *owner, EraseMode e, ObjListMode o)
    : mOwner(owner), mEraseMode(e), mMode(o) {
    MILO_ASSERT(owner, 0x321);
}

template <class T1, class T2>
ObjPtrVec<T1, T2>::ObjPtrVec(const ObjPtrVec &other)
    : mOwner(other.mOwner), mEraseMode(other.mEraseMode), mMode(other.mMode) {
    *this = other;
}

template <class T1, class T2>
ObjPtrVec<T1, T2>::Node::Node(const Node &n)
    : ObjRefConcrete<T1, T2>(n), parent(n.parent) {}

template <class T1, class T2>
ObjPtrVec<T1, T2>::~ObjPtrVec() {
    mVec.clear();
}

template <class T1, class T2>
void ObjPtrVec<T1, T2>::ReplaceNode(Node *n, Hmx::Object *obj) {
    if (mMode == kObjListOwnerControl) {
        mOwner->Replace(n, obj);
    } else {
        Hmx::Object *oldObj = n->SetObj(obj);
        if (!oldObj && mMode == kObjListNoNull) {
            erase(n);
        }
    }
}

template <class T1, class T2>
__declspec(noinline) void ObjPtrVec<T1, T2>::Set(iterator it, T1 *obj) {
    if (!obj && mMode == 0) {
        erase(it);
    } else {
        Node *itNode = *reinterpret_cast<Node **>(&it);
        itNode->SetObjConcrete(obj);
    }
}

template <class T1, class T2>
void ObjPtrVec<T1, T2>::operator=(const ObjPtrVec &other) {
    if (this != &other) {
        mVec.clear();
        mVec.reserve(other.mVec.size());
        for (const_iterator it = other.begin(); it != other.end(); ++it) {
            Node n(this);
            mVec.push_back(n);
            Set(--end(), *it);
        }
    }
}

template <class T1, class T2>
void ObjPtrVec<T1, T2>::push_back(T1 *obj) {
    insert(end(), obj);
}

template <class T1, class T2>
typename ObjPtrVec<T1, T2>::iterator
ObjPtrVec<T1, T2>::insert(typename ObjPtrVec<T1, T2>::const_iterator it, T1 *obj) {
    if (obj || mMode != kObjListNoNull) {
        Node *itNode = *reinterpret_cast<Node **>(&it);
        unsigned int idx = it != nullptr ? (itNode - mVec.begin()) : 0;
        Node n(this);
        mVec.insert(mVec.begin() + idx, n);
        Set(begin() + idx, obj);
    }
    return reinterpret_cast<iterator &>(it);
}

template <class T1, class T2>
typename ObjPtrVec<T1, T2>::iterator
ObjPtrVec<T1, T2>::erase(typename ObjPtrVec<T1, T2>::iterator it) {
    unsigned int idx = it != nullptr ? (it - begin()) : 0;
    if (mEraseMode == 1 && idx != size() - 1) {
        T1 *n = mVec.back();
        mVec.pop_back();
        Set(begin() + idx, n);
    } else {
        mVec.erase(mVec.begin() + idx);
    }
    return it;
}

template <class T1, class T2>
typename ObjPtrVec<T1, T2>::const_iterator
ObjPtrVec<T1, T2>::find(const Hmx::Object *target) const {
    auto it = begin();
    for (; it != end(); ++it) {
        if (*it == target) {
            break;
        }
    }
    return it;
}

template <class T1, class T2>
bool ObjPtrVec<T1, T2>::remove(T1 *obj) {
    const_iterator found = find(obj);
    if (found != cend()) {
        erase(reinterpret_cast<iterator &>(found));
        return true;
    } else {
        return false;
    }
}

template <class T1, class T2>
typename ObjPtrVec<T1, T2>::iterator ObjPtrVec<T1, T2>::FindRef(ObjRef *ref) {
    ObjRefOwner *parent = ref->Parent();
    if (parent == this) {
        return static_cast<Node *>(ref);
    } else {
        return end();
    }
}

template <class T1, class T2>
bool ObjPtrVec<T1, T2>::Load(BinStream &bs, bool print, ObjectDir *dir) {
    bool ret = true;
    mVec.clear();
    int count;
    bs >> count;
    mVec.reserve(count);
    if (!dir && mOwner) {
        dir = mOwner->Dir();
    }
    if (print) {
        MILO_ASSERT(dir, 0x488);
    }
    while (count != 0U) {
        char buf[0x80];
        bs.ReadString(buf, 0x80);
        if (dir) {
            T1 *casted = dynamic_cast<T1 *>(dir->FindObject(buf, false, true));
            if (!casted && buf[0] != '\0') {
                if (print)
                    MILO_NOTIFY(
                        "%s couldn't find %s in %s", PathName(mOwner), buf, PathName(dir)
                    );
                ret = false;
            } else if (casted || mMode != kObjListNoNull) {
                push_back(casted);
            }
        }
        count--;
    }
    return ret;
}

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjPtrVec<T1, ObjectDir> &c) {
    bs << c.size();
    MILO_ASSERT(c.Owner(), 0x525);
    for (auto it = c.begin(); it != c.end(); ++it) {
        if (*it) {
            bs << (*it)->Name();
        } else {
            bs << "";
        }
    }
    return bs;
}

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrVec<T1, ObjectDir> &vec) {
    vec.Load(bs, true, nullptr);
    return bs;
}

#pragma endregion
#pragma region ObjPtrList
// ------------------------------------------------
// ObjPtrList
// ------------------------------------------------

template <class T1, class T2>
ObjPtrList<T1, T2>::ObjPtrList(ObjRefOwner *owner, ObjListMode mode)
    : mSize(0), mNodes(nullptr), mOwner(owner), mMode(mode) {
    if (mode == kObjListOwnerControl) {
        MILO_ASSERT(owner, 0x103);
    }
}

template <class T1, class T2>
ObjPtrList<T1, T2>::ObjPtrList(const ObjPtrList &other)
    : mSize(0), mNodes(nullptr), mOwner(other.mOwner), mMode(other.mMode) {
    for (iterator it = other.begin(); it != other.end(); ++it) {
        push_back(*it);
    }
}

template <class T1, class T2>
void *ObjPtrList<T1, T2>::Node::operator new(unsigned int s) {
    return PoolAlloc(s, s, __FILE__, 0x122, "ObjPtrList_node");
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::Node::operator delete(void *v) {
    PoolFree(sizeof(Node), v, __FILE__, 0x122, "ObjPtrList_node");
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::ReplaceNode(struct ObjPtrList::Node *node, Hmx::Object *obj) {
    if (mMode == kObjListOwnerControl) {
        mOwner->Replace(node, obj);
    } else if (!node->SetObj(obj) && mMode == kObjListNoNull) {
        erase(node);
    }
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::operator=(const ObjPtrList &other) {
    if (this == &other)
        return;
    while (mSize > other.mSize)
        pop_back();
    Node *otherNodes = other.mNodes;
    for (Node *n = mNodes; n != nullptr; n = n->next, otherNodes = otherNodes->next) {
        *n = *otherNodes;
    }
    for (; otherNodes != nullptr; otherNodes = otherNodes->next) {
        push_back(*otherNodes);
    }
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::Link(iterator it, Node *n) {
    n->parent = this;
    Node *itNode = *reinterpret_cast<Node **>(&it);
    n->next = itNode;
    if (n->next == mNodes) {
        if (mNodes) {
            n->prev = mNodes->prev;
            mNodes->prev = n;
        } else {
            n->prev = n;
        }
        mNodes = n;
    } else if (n->next == nullptr) {
        n->prev = mNodes->prev;
        mNodes->prev->next = n;
        mNodes->prev = n;
    } else {
        n->prev = itNode->prev;
        itNode->prev->next = n;
        itNode->prev = n;
    }
    mSize++;
}

template <class T1, class T2>
typename ObjPtrList<T1, T2>::Node *ObjPtrList<T1, T2>::Unlink(Node *n) {
    MILO_ASSERT(n != NULL && mNodes != NULL, 0x26B);
    if (n == mNodes) {
        if (mNodes->next) {
            mNodes->next->prev = mNodes->prev;
            mNodes = mNodes->next;
        } else {
            mNodes = nullptr;
        }
        n = mNodes;
    } else if (n == mNodes->prev) {
        mNodes->prev = mNodes->prev->prev;
        mNodes->prev->next = nullptr;
        n = mNodes->prev;
    } else {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        n = n->next;
    }
    mSize--;
    return n;
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::pop_back() {
    MILO_ASSERT(mNodes != NULL, 0x18B);
    erase(mNodes->prev);
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::pop_front() {
    MILO_ASSERT(mNodes != NULL, 0x18C);
    erase(mNodes);
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::push_back(T1 *obj) {
    insert(end(), obj);
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::push_front(T1 *obj) {
    insert(begin(), obj);
}

template <class T1, class T2>
T1 *ObjPtrList<T1, T2>::front() const {
    MILO_ASSERT(mNodes != NULL, 0x189);
    // stupid way of getting the underlying T1*
    // the operator T1*() SHOULD work, but it isn't, and I don't wanna add a Obj() method
    // so here you go. don't like it? cry more
    return mNodes->operator->();
}

template <class T1, class T2>
T1 *ObjPtrList<T1, T2>::back() const {
    MILO_ASSERT(mNodes != NULL, 0x18A);
    // stupid way of getting the underlying T1*
    // the operator T1*() SHOULD work, but it isn't, and I don't wanna add a Obj() method
    // so here you go. don't like it? cry more
    return mNodes->prev->operator->();
}

template <class T1, class T2>
typename ObjPtrList<T1, T2>::iterator
ObjPtrList<T1, T2>::insert(typename ObjPtrList<T1, T2>::iterator it, T1 *obj) {
    if (mMode == kObjListNoNull) {
        MILO_ASSERT(obj, 0x177);
    }
    Node *node = new Node();
    node->SetObjConcrete(obj);
    Link(it, node);
    return node;
}

template <class T1, class T2>
void ObjPtrList<T1, T2>::Set(iterator it, T1 *obj) {
    Node *n = *reinterpret_cast<Node **>(&it);
    n->SetObjConcrete(obj);
}

template <class T1, class T2>
typename ObjPtrList<T1, T2>::iterator ObjPtrList<T1, T2>::erase(iterator it) {
    Node *n = *reinterpret_cast<Node **>(&it);
    Node *unlinked = Unlink(n);
    delete n;
    return unlinked;
}

template <class T1, class T2>
inline typename ObjPtrList<T1, T2>::iterator
ObjPtrList<T1, T2>::find(const Hmx::Object *target) const {
    for (iterator it = begin(); it != end(); ++it) {
        if (*it == target)
            return it;
    }
    return end();
}

template <class T1, class T2>
template <typename Cmp>
void ObjPtrList<T1, T2>::sort(const Cmp &cmp) {
    if (mNodes && mNodes->next) {
        Node *last = mNodes->prev;
        for (Node *n = last->prev; n != last; n = n->prev) {
            for (Node *x = n; x != last; x = x->next) {
                Node *nextX = x->next;
                if (cmp(*nextX, *x)) {
                    T1 *tmp = *x;
                    x->SetObjConcrete(*nextX);
                    nextX->SetObjConcrete(tmp);
                } else {
                    break;
                }
            }
        }
    }
}

template <class T1, class T2>
bool ObjPtrList<T1, T2>::remove(T1 *target) {
    for (Node *it = mNodes; it != nullptr;) {
        Node *old = it;
        it = it->next;
        if (*old == target) {
            erase(old);
            return true;
        }
    }
    return false;
}

// remove a particular item inside iterator otherIt, from list otherList,
// and insert it into this list at the position indicated by thisIt
template <class T1, class T2>
void ObjPtrList<T1, T2>::splice(
    iterator thisIt, ObjPtrList<T1, T2> &otherList, iterator otherIt
) {
    if (otherIt != thisIt) {
        Node *n = *reinterpret_cast<Node **>(&otherIt);
        otherList.Unlink(n);
        Link(thisIt, n);
    }
}

template <class T1, class T2>
bool ObjPtrList<T1, T2>::Load(BinStream &bs, bool print, ObjectDir *dir, bool b4) {
    bool ret = true;
    clear();
    int count;
    bs >> count;
    Hmx::Object *refOwner = mOwner ? mOwner->RefOwner() : nullptr;
    if (!dir && refOwner)
        dir = refOwner->Dir();
    if (print) {
        MILO_ASSERT(dir, 0x210);
    }
    while (count != 0U) {
        char buf[0x80];
        bs.ReadString(buf, 0x80);
        if (dir) {
            T1 *casted = dynamic_cast<T1 *>(dir->FindObject(buf, false, b4));
            if (!casted && buf[0] != '\0') {
                if (print)
                    MILO_NOTIFY(
                        "%s couldn't find %s in %s", PathName(refOwner), buf, PathName(dir)
                    );
                ret = false;
            } else if (casted) {
                push_back(casted);
            }
        }
        count--;
    }
    return ret;
}

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjPtrList<T1, ObjectDir> &c) {
    bs << c.size();
    MILO_ASSERT(c.Owner(), 0x4E1);
    FOREACH (it, c) {
        if (*it) {
            bs << (*it)->Name();
        } else {
            bs << "";
        }
    }
    return bs;
}

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrList<T1, ObjectDir> &list) {
    list.Load(bs, true, nullptr, true);
    return bs;
}
