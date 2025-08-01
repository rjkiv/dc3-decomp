#pragma once
#include "obj/ObjRef.h"
#include "os/Debug.h"
#include <cstddef>
#include <vector>

// ObjPtr size: 0x14
template <class T>
class ObjPtr : public ObjRefConcrete<T, ObjectDir> {
private:
    Hmx::Object *mOwner; // 0x10
public:
    ObjPtr(Hmx::Object *owner, T *ptr)
        : ObjRefConcrete<T, ObjectDir>(ptr), mOwner(owner) {}
    ObjPtr(const ObjPtr &p);
    virtual ~ObjPtr() {}
    virtual Hmx::Object *RefOwner() const { return mOwner; }
};

// ObjOwnerPtr size: 0x14
template <class T>
class ObjOwnerPtr : public ObjRefConcrete<T, ObjectDir> {
private:
    ObjRefOwner *mOwner; // 0x10
public:
    ObjOwnerPtr(ObjRefOwner *owner, T *ptr);
    virtual ~ObjOwnerPtr() {}
    virtual Hmx::Object *RefOwner() const { return mObj->RefOwner(); }
    virtual void Replace(Hmx::Object *obj) { mOwner->Replace(this, obj); }
};

enum EraseMode {
};

enum ObjListMode {
    kObjListNoNull,
    kObjListAllowNull,
    kObjListOwnerControl
};

// ObjPtrVec size: 0x1c
template <class T1, class T2 = class ObjectDir>
class ObjPtrVec : public ObjRefOwner {
    // Node size: 0x14
    struct Node : public ObjRefConcrete<T1, T2> {
        virtual ~Node() {}
        virtual Hmx::Object *RefOwner() const;
        virtual void Replace(Hmx::Object *);
        virtual ObjRefOwner *Parent() const { return unk10; }

        ObjRefOwner *unk10; // 0x10
    };
    virtual Hmx::Object *RefOwner() const { return mOwner; }
    virtual bool Replace(ObjRef *, Hmx::Object *);

public:
    ObjPtrVec(Hmx::Object *owner, EraseMode, ObjListMode);

private:
    std::vector<Node> mNodes; // 0x4
    Hmx::Object *mOwner; // 0x10
    EraseMode mEraseMode; // 0x14
    ObjListMode mListMode; // 0x18
};

// ObjPtrList size: 0x14
template <class T1, class T2 = class ObjectDir>
class ObjPtrList : public ObjRefOwner {
public:
    ObjPtrList(ObjRefOwner *, ObjListMode);
    virtual ~ObjPtrList() { clear(); }

private:
    // Node size: 0x14
    struct Node : public ObjRefConcrete<T1, T2> {
        virtual ~Node() {}
        virtual Hmx::Object *RefOwner() const;
        virtual void Replace(Hmx::Object *);
        virtual ObjRefOwner *Parent() const { return unk10; }

        ObjRefOwner *unk10; // 0x10
        Node *next; // 0x14
        Node *prev; // 0x18
    };
    int mSize; // 0x4
    Node *mNodes; // 0x8
    ObjRefOwner *mOwner; // 0xc
    ObjListMode mListMode; // 0x10

    virtual Hmx::Object *RefOwner() const;
    virtual bool Replace(ObjRef *, Hmx::Object *);

    Node *Unlink(Node *);

public:
    class iterator {
    public:
        iterator() : mNode(0) {}
        iterator(Node *node) : mNode(node) {}
        T1 *operator*() { return mNode->obj; }

        iterator operator++() {
            mNode = mNode->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator!=(iterator it) { return mNode != it.mNode; }
        bool operator==(iterator it) { return mNode == it.mNode; }
        bool operator!() { return mNode == 0; }

        struct Node *mNode;
    };

    void clear() {
        while (mSize != 0)
            pop_back();
    }

    void pop_back() {
        MILO_ASSERT(mNodes != NULL, 0x18B);
        erase(mNodes->prev);
    }

    void push_back(T1 *obj) { insert(end(), obj); }

    iterator begin() const { return iterator(mNodes); }
    iterator end() const { return iterator(0); }
    iterator erase(iterator);
    iterator insert(iterator, Hmx::Object *);
};

// // ObjDirPtr size: 0x14
// template <class T>
// class ObjDirPtr : public ObjRefConcrete<T, ObjectDir> {
//     DirLoader* mLoader; // 0x10
// };

// public: class ObjPtrList<class Hmx::Object, class ObjectDir>::iterator
//      __cdecl ObjPtrList<class Hmx::Object, class ObjectDir>::insert(class
//      ObjPtrList<class Hmx::Object, class ObjectDir>::iterator, class Hmx::Object *)
