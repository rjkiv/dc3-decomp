#pragma once
#include "obj/Data.h" /* IWYU pragma: keep */
#include "obj/PropSync.h" /* IWYU pragma: keep */
#include "obj/MessageTimer.h" /* IWYU pragma: keep */
#include "utl/BinStream.h" /* IWYU pragma: keep */
#include "utl/MemMgr.h" /* IWYU pragma: keep */
#include "utl/Symbol.h" /* IWYU pragma: keep */
#include <list> /* IWYU pragma: keep */
#include <map> /* IWYU pragma: keep */

// forward declarations
class MsgSinks;
namespace Hmx {
    class Object;
}
class ObjRef;
class ObjRefOwner;
class ObjectDir;

// Object Ref/Ptr declarations
// ObjRefOwner size: 0x4
class ObjRefOwner {
public:
    ObjRefOwner() {}
    virtual ~ObjRefOwner() {}
    virtual Hmx::Object *RefOwner() const = 0;
    virtual bool Replace(ObjRef *, Hmx::Object *) = 0;
};

// ObjRef size: 0xc
class ObjRef {
    friend class Hmx::Object;

protected:
    // seems to be a linked list of an Object's refs
    ObjRef *next; // 0x4
    ObjRef *prev; // 0x8

    // i *think* this is good?
    void AddRef(ObjRef *ref) {
        next = ref;
        prev = ref->prev;
        ref->prev = this;
        prev->next = this;
    }

    void Release(ObjRef *ref) {
        prev->next = next;
        next->prev = prev;
        // do something with ref here
    }

public:
    ObjRef() {}
    ObjRef(const ObjRef &other) : next(other.next), prev(other.prev) {
        prev->next = this;
        next->prev = this;
    }
    virtual ~ObjRef() {}
    virtual Hmx::Object *RefOwner() const { return nullptr; }
    virtual bool IsDirPtr() { return false; }
    virtual Hmx::Object *GetObj() const {
        MILO_FAIL("calling get obj on abstract ObjRef");
        return nullptr;
    }
    virtual void Replace(Hmx::Object *) {
        MILO_FAIL("calling get obj on abstract ObjRef");
    }
    virtual ObjRefOwner *Parent() const { return nullptr; }

    class iterator {
    private:
        ObjRef *curRef;

    public:
        iterator() : curRef(nullptr) {}
        iterator(ObjRef *ref) : curRef(ref) {}
        operator ObjRef *() const { return curRef; }
        ObjRef *operator->() const { return curRef; }

        iterator operator++() {
            curRef = curRef->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator!=(iterator it) { return curRef != it.curRef; }
        bool operator==(iterator it) { return curRef == it.curRef; }
        bool operator!() { return curRef == nullptr; }
    };

    iterator begin() const { return iterator(next); }
    iterator end() const { return iterator((ObjRef *)this); }

    void Clear() { next = prev = this; }
    void ReplaceList(Hmx::Object *obj) {
        while (next != this) {
            next->Replace(obj);
            if (this == next) {
                MILO_FAIL("ReplaceList stuck in infinite loop");
            }
        }
    }
    int RefCount() const {
        int count = 0;
        for (ObjRef::iterator it = begin(); it != end(); ++it) {
            count++;
        }
        return count;
    }

    // per ObjectDir::HasDirPtrs, this is the way to iterate across refs
    // for (ObjRef *it = mRefs.next; it != &mRefs; it = it->next) {
};

// ObjRefConcrete size: 0x10
template <class T1, class T2 = class ObjectDir>
class ObjRefConcrete : public ObjRef {
protected:
    T1 *mObject; // 0xc
public:
    ObjRefConcrete(T1 *obj);
    ObjRefConcrete(const ObjRefConcrete &o);
    virtual ~ObjRefConcrete();
    virtual Hmx::Object *GetObj() const { return mObject; }
    virtual void Replace(Hmx::Object *obj) { SetObj(obj); }

    T1 *operator->() const { return mObject; }
    operator T1 *() const { return mObject; }
    void operator=(T1 *obj) { SetObjConcrete(obj); }
    void operator=(const ObjRefConcrete &o) { SetObjConcrete(o); }

    void SetObjConcrete(T1 *obj);
    void CopyRef(const ObjRefConcrete &);
    Hmx::Object *SetObj(Hmx::Object *root_obj);
    bool Load(BinStream &, bool, ObjectDir *);
};

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjRefConcrete<T1, class ObjectDir> &f);

// ObjPtr size: 0x14
template <class T>
class ObjPtr : public ObjRefConcrete<T> {
private:
    Hmx::Object *mOwner; // 0x10
public:
    ObjPtr(Hmx::Object *owner, T *ptr = nullptr);
    ObjPtr(const ObjPtr &p);
    virtual ~ObjPtr();
    virtual Hmx::Object *RefOwner() const { return mOwner; }

    void operator=(T *obj) { SetObjConcrete(obj); }
    void operator=(const ObjPtr &p) { CopyRef(p); }
    T *Ptr() const { return mObject; }
    Hmx::Object *Owner() const { return mOwner; }
};

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjPtr<T1> &ptr);

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtr<T1> &ptr);

// ObjOwnerPtr size: 0x14
template <class T>
class ObjOwnerPtr : public ObjRefConcrete<T> {
private:
    ObjRefOwner *mOwner; // 0x10
public:
    ObjOwnerPtr(ObjRefOwner *owner, T *ptr = nullptr);
    ObjOwnerPtr(const ObjOwnerPtr &o);
    virtual ~ObjOwnerPtr();
    virtual Hmx::Object *RefOwner() const;
    virtual void Replace(Hmx::Object *obj) { mOwner->Replace(this, obj); }
    void operator=(T *obj) { SetObjConcrete(obj); }
    T *Ptr() const { return mObject; }
};

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjOwnerPtr<T1> &ptr);

template <class T1>
BinStream &operator>>(BinStream &bs, ObjOwnerPtr<T1> &ptr);

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
private:
    // Node size: 0x14
    struct Node : public ObjRefConcrete<T1, T2> {
        Node(ObjRefOwner *owner) : ObjRefConcrete<T1>(nullptr), mOwner(owner) {}
        Node(const Node &n);
        virtual ~Node() {}
        virtual Hmx::Object *RefOwner() const;
        virtual void Replace(Hmx::Object *obj) {
            ObjPtrVec<T1, T2> *vec = static_cast<ObjPtrVec<T1, T2> *>(mOwner);
            vec->ReplaceNode(this, obj);
        }
        virtual ObjRefOwner *Parent() const { return mOwner; }

        T1 *Obj() const { return mObject; }

        /** The ObjPtrVec this Node belongs to. */
        ObjRefOwner *mOwner; // 0x10
    };
    virtual Hmx::Object *RefOwner() const { return mOwner; }
    virtual bool Replace(ObjRef *, Hmx::Object *);

protected:
    void ReplaceNode(Node *, Hmx::Object *);

public:
    // this derives off of std::vector<Node>::iterator in some way
    class iterator {
    private:
        typedef typename std::vector<Node>::iterator Base;
        Base it;

    public:
        iterator(Base base) : it(base) {}

        Node &operator*() const { return *it; }
        Node *operator->() const { return &(*it); }

        iterator &operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const iterator &other) const { return it != other.it; }
    };
    // ditto
    class const_iterator {
    private:
        typedef typename std::vector<Node>::const_iterator Base;
        Base it;

    public:
        const_iterator(Base base) : it(base) {}

        const Node &operator*() const { return *it; }
        const Node *operator->() const { return &(*it); }

        const_iterator &operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const const_iterator &other) const { return it != other.it; }
    };

    ObjPtrVec(Hmx::Object *owner, EraseMode = (EraseMode)0, ObjListMode = kObjListNoNull);
    ObjPtrVec(const ObjPtrVec &);
    virtual ~ObjPtrVec();

    iterator begin() { return iterator(mNodes.begin()); }
    iterator end() { return iterator(mNodes.end()); }
    const_iterator begin() const { return const_iterator(mNodes.begin()); }
    const_iterator end() const { return const_iterator(mNodes.end()); }

    iterator erase(iterator);
    iterator insert(const_iterator, T1 *);
    const_iterator find(const Hmx::Object *) const;
    int size() const { return mNodes.size(); }
    bool empty() const { return mNodes.empty(); }
    T1 *front() const { return mNodes.front(); }
    T1 *operator[](int idx) { return mNodes[idx].Obj(); }
    const T1 *operator[](int idx) const { return mNodes[idx].Obj(); }

    template <class S>
    void sort(const S &);

    bool remove(T1 *);
    void push_back(T1 *);
    void swap(int, int);
    bool Load(BinStream &, bool, ObjectDir *);
    void clear() { mNodes.clear(); }
    void reserve(unsigned int n) { mNodes.reserve(n); }
    void unique();
    void Set(iterator it, T1 *obj);

    // see Draw.cpp for this
    void operator=(const ObjPtrVec &other);

private:
    std::vector<Node> mNodes; // 0x4
    Hmx::Object *mOwner; // 0x10
    EraseMode mEraseMode; // 0x14
    ObjListMode mListMode; // 0x18
};

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjPtrVec<T1, ObjectDir> &vec);

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrVec<T1, ObjectDir> &vec);

// ObjPtrList size: 0x14
template <class T1, class T2 = class ObjectDir>
class ObjPtrList : public ObjRefOwner {
public:
    ObjPtrList(ObjRefOwner *, ObjListMode = kObjListNoNull);
    virtual ~ObjPtrList() { clear(); }

private:
    // Node size: 0x14
    struct Node : public ObjRefConcrete<T1, T2> {
        Node() : ObjRefConcrete(nullptr) {}
        virtual ~Node() {}
        virtual Hmx::Object *RefOwner() const;
        virtual void Replace(Hmx::Object *obj) {
            ObjPtrList<T1, T2> *list = static_cast<ObjPtrList<T1, T2> *>(mOwner);
            list->ReplaceNode(this, obj);
        }
        virtual ObjRefOwner *Parent() const { return mOwner; }

        static void *operator new(unsigned int);
        static void operator delete(void *);

        T1 *Obj() const { return mObject; }
        void operator=(const Node &n) { SetObjConcrete(n.mObject); }

        ObjRefOwner *mOwner; // 0x10
        Node *next; // 0x14
        Node *prev; // 0x18
    };
    int mSize; // 0x4
    Node *mNodes; // 0x8
    ObjRefOwner *mOwner; // 0xc
    ObjListMode mListMode; // 0x10

    virtual Hmx::Object *RefOwner() const;
    virtual bool Replace(ObjRef *, Hmx::Object *);
    void ReplaceNode(Node *, Hmx::Object *);

public:
    class iterator {
    public:
        iterator() : mNode(0) {}
        iterator(Node *node) : mNode(node) {}
        T1 *operator*() { return mNode->Obj(); }

        iterator operator++() {
            mNode = mNode->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        // iterator &operator=(T1 *obj) {
        //     mNode->SetObjConcrete(obj);
        //     return *this;
        // }

        bool operator!=(iterator it) { return mNode != it.mNode; }
        bool operator==(iterator it) { return mNode == it.mNode; }
        bool operator!() { return mNode == 0; }

        struct Node *mNode; // 0x0
    };

    ObjListMode Mode() const { return mListMode; }
    int size() const { return mSize; }
    bool empty() const { return mSize == 0; }

    void clear() {
        while (mSize != 0)
            pop_back();
    }

    void DeleteAll() {
        while (!empty()) {
            T1 *cur = front();
            pop_front();
            delete cur;
        }
    }

    T1 *front() const;
    void pop_front();
    void pop_back();
    void push_back(T1 *obj);
    iterator find(const Hmx::Object *target) const;
    iterator begin() const { return iterator(mNodes); }
    iterator end() const { return iterator(0); }
    iterator erase(iterator);
    iterator insert(iterator, T1 *);
    void Set(iterator it, T1 *obj);

    typedef bool SortFunc(T1 *, T1 *);
    void sort(SortFunc *func);

    void operator=(const ObjPtrList &list);
    bool remove(T1 *);
    bool Load(BinStream &bs, bool, ObjectDir *, bool);

private:
    void Link(iterator, Node *);
    Node *Unlink(Node *);
};

template <class T1>
BinStream &operator<<(BinStream &bs, const ObjPtrList<T1, ObjectDir> &list);

template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrList<T1, ObjectDir> &list);

// Hmx::Object-centric macros
/** Get this Object's path name.
 * @param [in] obj The Object.
 * @returns The Object's path name, or "NULL Object" if it doesn't exist.
 */
const char *PathName(const class Hmx::Object *obj);

#define NULL_OBJ (Hmx::Object *)nullptr

// BEGIN CLASSNAME MACRO -----------------------------------------------------------------
#define OBJ_CLASSNAME(classname)                                                         \
    virtual Symbol ClassName() const { return StaticClassName(); }                       \
    static Symbol StaticClassName() {                                                    \
        static Symbol name(#classname);                                                  \
        return name;                                                                     \
    }
// END CLASSNAME MACRO -------------------------------------------------------------------

// BEGIN SET TYPE MACRO ------------------------------------------------------------------
extern DataArray *SystemConfig(Symbol, Symbol, Symbol);
#define OBJ_SET_TYPE(classname)                                                           \
    virtual void SetType(Symbol classname) {                                              \
        DataArray *def;                                                                   \
        if (!classname.Null()) {                                                          \
            static DataArray *types =                                                     \
                SystemConfig("objects", StaticClassName(), "types");                      \
            DataArray *found = types->FindArray(classname, false);                        \
            if (found) {                                                                  \
                SetTypeDef(found);                                                        \
            } else {                                                                      \
                MILO_NOTIFY(                                                              \
                    "%s:%s couldn't find type %s", ClassName(), PathName(this), classname \
                );                                                                        \
                SetTypeDef(nullptr);                                                      \
            }                                                                             \
        } else                                                                            \
            SetTypeDef(nullptr);                                                          \
    }
// END SET TYPE MACRO --------------------------------------------------------------------

// BEGIN HANDLE MACROS -------------------------------------------------------------------
#define BEGIN_HANDLERS(objType)                                                          \
    DataNode objType::Handle(DataArray *_msg, bool _warn) {                              \
        Symbol sym = _msg->Sym(1);                                                       \
        MessageTimer timer(                                                              \
            (MessageTimer::Active()) ? static_cast<Hmx::Object *>(this) : 0, sym         \
        );

// for handlers of objects that aren't directly Hmx::Objects (i.e. UIListProvider)
#define BEGIN_CUSTOM_HANDLERS(objType)                                                   \
    DataNode objType::Handle(DataArray *_msg, bool _warn) {                              \
        Symbol sym = _msg->Sym(1);                                                       \
        MessageTimer timer(                                                              \
            (MessageTimer::Active()) ? dynamic_cast<Hmx::Object *>(this) : 0, sym        \
        );

#define _NEW_STATIC_SYMBOL(str) static Symbol _s(#str);

#define _HANDLE_CHECKED(expr)                                                            \
    {                                                                                    \
        DataNode result = expr;                                                          \
        if (result.Type() != kDataUnhandled)                                             \
            return result;                                                               \
    }

#define HANDLE(s, func)                                                                  \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            _HANDLE_CHECKED(func(_msg))                                                  \
    }

#define HANDLE_EXPR(s, expr)                                                             \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            return expr;                                                                 \
    }

#define HANDLE_ACTION(s, action)                                                         \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            /* for style, require any side-actions to be performed via comma operator */ \
            (action);                                                                    \
            return 0;                                                                    \
        }                                                                                \
    }

#define HANDLE_ACTION_IF(s, cond, action)                                                \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (cond) {                                                                  \
                /* for style, require any side-actions to be performed via comma         \
                 * operator */                                                           \
                (action);                                                                \
            }                                                                            \
            return 0;                                                                    \
        }                                                                                \
    }

#define HANDLE_ARRAY(array)                                                              \
    {                                                                                    \
        /* this needs to be placed up here to match Hmx::Object::Handle */               \
        DataArray *found;                                                                \
        if (array && (found = array->FindArray(sym, false))) {                           \
            _HANDLE_CHECKED(found->ExecuteScript(1, this, _msg, 2))                      \
        }                                                                                \
    }

#define HANDLE_MESSAGE(msg)                                                              \
    if (sym == msg::Type())                                                              \
    _HANDLE_CHECKED(OnMsg(msg(_msg)))

#define HANDLE_FORWARD(func) _HANDLE_CHECKED(func(_msg, false))

#define HANDLE_MEMBER(member) HANDLE_FORWARD(member.Handle)

#define HANDLE_MEMBER_PTR(member)                                                        \
    if (member)                                                                          \
    HANDLE_FORWARD(member->Handle)

#define HANDLE_SUPERCLASS(parent) HANDLE_FORWARD(parent::Handle)

#define HANDLE_VIRTUAL_SUPERCLASS(parent)                                                \
    if (ClassName() == StaticClassName())                                                \
    HANDLE_SUPERCLASS(parent)

#define END_HANDLERS                                                                     \
    if (_warn)                                                                           \
        MILO_NOTIFY("%s unhandled msg: %s", PathName(this), sym);                        \
    return DataNode(kDataUnhandled, 0);                                                  \
    }

// for handlers of objects that aren't directly Hmx::Objects (i.e. UIListProvider)
#define END_CUSTOM_HANDLERS                                                              \
    if (_warn)                                                                           \
        MILO_NOTIFY(                                                                     \
            "%s unhandled msg: %s", PathName(dynamic_cast<Hmx::Object *>(this)), sym     \
        );                                                                               \
    return DataNode(kDataUnhandled, 0);                                                  \
    }
// END HANDLE MACROS ---------------------------------------------------------------------

// BEGIN SYNCPROPERTY MACROS -------------------------------------------------------------
#define BEGIN_PROPSYNCS(objType)                                                         \
    bool objType::SyncProperty(DataNode &_val, DataArray *_prop, int _i, PropOp _op) {   \
        if (_i == _prop->Size())                                                         \
            return true;                                                                 \
        else {                                                                           \
            Symbol sym = _prop->Sym(_i);

#define BEGIN_CUSTOM_PROPSYNC(objType)                                                   \
    bool PropSync(objType &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op) {    \
        if (_i == _prop->Size())                                                         \
            return true;                                                                 \
        else {                                                                           \
            Symbol sym = _prop->Sym(_i);

#define SYNC_PROP(s, member)                                                             \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            return PropSync(member, _val, _prop, _i + 1, _op);                           \
    }

// for propsyncs that do something extra if the prop op is specifically kPropSet
#define SYNC_PROP_SET(s, member, func)                                                   \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (_op == kPropSet) {                                                       \
                func;                                                                    \
            } else {                                                                     \
                if (_op == (PropOp)0x40)                                                 \
                    return false;                                                        \
                _val = member;                                                           \
            }                                                                            \
            return true;                                                                 \
        }                                                                                \
    }

// for propsyncs that do NOT use size or get - aka, any combo of set, insert, remove, and
// handle is used
#define SYNC_PROP_MODIFY(s, member, func)                                                \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (PropSync(member, _val, _prop, _i + 1, _op)) {                            \
                if (!(_op & (kPropSize | kPropGet))) {                                   \
                    func;                                                                \
                }                                                                        \
                return true;                                                             \
            } else {                                                                     \
                return false;                                                            \
            }                                                                            \
        }                                                                                \
    }

#define SYNC_PROP_BITFIELD(symbol, mask_member, line_num)                                \
    if (sym == symbol) {                                                                 \
        _i++;                                                                            \
        if (_i < _prop->Size()) {                                                        \
            DataNode &node = _prop->Node(_i);                                            \
            int res = 0;                                                                 \
            switch (node.Type()) {                                                       \
            case kDataInt:                                                               \
                res = node.Int();                                                        \
                break;                                                                   \
            case kDataSymbol: {                                                          \
                const char *bitstr = node.Sym().Str();                                   \
                MILO_ASSERT_FMT(                                                         \
                    strncmp("BIT_", bitstr, 4) == 0,                                     \
                    "%s does not begin with BIT_",                                       \
                    bitstr                                                               \
                );                                                                       \
                Symbol bitsym(bitstr + 4);                                               \
                const Symbol &test = Symbol(bitsym);                                     \
                DataArray *macro = DataGetMacro(test);                                   \
                MILO_ASSERT_FMT(                                                         \
                    macro, "PROPERTY_BITFIELD %s could not find macro %s", symbol, test  \
                );                                                                       \
                res = macro->Int(0);                                                     \
                break;                                                                   \
            }                                                                            \
            default:                                                                     \
                MILO_ASSERT(0, line_num);                                                 \
                break;                                                                   \
            }                                                                            \
            MILO_ASSERT(_op <= kPropInsert, line_num);                                      \
            if (_op == kPropGet) {                                                       \
                int final = mask_member & res;                                           \
                _val = DataNode(final > 0);                                              \
            } else {                                                                     \
                if (_val.Int() != 0)                                                     \
                    mask_member |= res;                                                  \
                else                                                                     \
                    mask_member &= ~res;                                                 \
            }                                                                            \
            return true;                                                                 \
        } else                                                                           \
            return PropSync(mask_member, _val, _prop, _i, _op);                          \
    }

#define SYNC_PROP_MODIFY_STATIC(symbol, member, func)                                    \
    { _NEW_STATIC_SYMBOL(symbol) SYNC_PROP_MODIFY(_s, member, func) }

#define SYNC_PROP_BITFIELD_STATIC(symbol, mask_member, line_num)                         \
    { _NEW_STATIC_SYMBOL(symbol) SYNC_PROP_BITFIELD(_s, mask_member, line_num) }

#define SYNC_SUPERCLASS(parent)                                                          \
    if (parent::SyncProperty(_val, _prop, _i, _op))                                      \
        return true;

#define SYNC_VIRTUAL_SUPERCLASS(parent)                                                  \
    if (ClassName() == StaticClassName())                                                \
    SYNC_SUPERCLASS(parent)

#define END_PROPSYNCS                                                                    \
    return false;                                                                        \
    }                                                                                    \
    }

#define END_CUSTOM_PROPSYNC                                                              \
    return false;                                                                        \
    }                                                                                    \
    }

// END SYNCPROPERTY MACROS ---------------------------------------------------------------

// BEGIN SAVE MACROS ---------------------------------------------------------------------
#define BEGIN_SAVES(objType) void objType::Save(BinStream &bs) {
#define SAVE_REVS(rev, alt) bs << packRevs(alt, rev);
#define SAVE_SUPERCLASS(parent) parent::Save(bs);

#define SAVE_VIRTUAL_SUPERCLASS(parent)                                                  \
    if (ClassName() == StaticClassName())                                                \
    SAVE_SUPERCLASS(parent)

#define END_SAVES }
// END SAVE MACRO ------------------------------------------------------------------------

// BEGIN COPY MACROS ---------------------------------------------------------------------
#define BEGIN_COPYS(objType)                                                             \
    void objType::Copy(const Hmx::Object *o, Hmx::Object::CopyType ty) {
#define COPY_SUPERCLASS(parent) parent::Copy(o, ty);

#define COPY_VIRTUAL_SUPERCLASS(parent)                                                  \
    if (ClassName() == StaticClassName())                                                \
    COPY_SUPERCLASS(Hmx::Object)

#define COPY_SUPERCLASS_FROM(parent, obj) parent::Copy(obj, ty);

#define CREATE_COPY(objType) const objType *c = dynamic_cast<const objType *>(o);

// copy macro where you specify the variable name (used in asserts in some copy methods)
#define CREATE_COPY_AS(objType, var_name)                                                \
    const objType *var_name = dynamic_cast<const objType *>(o);

#define BEGIN_COPYING_MEMBERS if (c) {
// copy macro where you specify the variable name (used in asserts in some copy methods)
#define BEGIN_COPYING_MEMBERS_FROM(copy_name) if (copy_name) {
#define COPY_MEMBER(mem) mem = c->mem;

// copy macro where you specify the variable name (used in asserts in some copy methods)
#define COPY_MEMBER_FROM(copy_name, member) member = copy_name->member;

#define END_COPYING_MEMBERS }

#define END_COPYS }
// END COPY MACROS -----------------------------------------------------------------------

// BEGIN LOAD MACROS  --------------------------------------------------------------------
#define BEGIN_LOADS(objType) void objType::Load(BinStream &bs) {
#define LOAD_REVS(bs)                                                                    \
    int revs;                                                                            \
    bs >> revs;                                                                          \
    int gRev = getHmxRev(revs);                                                          \
    int gAltRev = getAltRev(revs);                                                       \
    BinStreamRev bsrev(bs, gRev, gAltRev);

#define ASSERT_REVS(rev1, rev2)                                                          \
    static const unsigned short gRevs[4] = { rev1, 0, rev2, 0 };                         \
    if (gRev > rev1) {                                                                   \
        MILO_FAIL(                                                                       \
            "%s can't load new %s version %d > %d",                                      \
            PathName(this),                                                              \
            ClassName(),                                                                 \
            gRev,                                                                        \
            gRevs[0]                                                                     \
        );                                                                               \
    }                                                                                    \
    if (gAltRev > rev2) {                                                                \
        MILO_FAIL(                                                                       \
            "%s can't load new %s alt version %d > %d",                                  \
            PathName(this),                                                              \
            ClassName(),                                                                 \
            gAltRev,                                                                     \
            gRevs[2]                                                                     \
        );                                                                               \
    }

#define LOAD_SUPERCLASS(parent) parent::Load(bs);

#define LOAD_VIRTUAL_SUPERCLASS(parent)                                                  \
    if (ClassName() == StaticClassName())                                                \
    LOAD_SUPERCLASS(parent)

#define LOAD_BITFIELD(type, name)                                                        \
    {                                                                                    \
        type bs_name;                                                                    \
        bs >> bs_name;                                                                   \
        name = bs_name;                                                                  \
    }

#define LOAD_BITFIELD_ENUM(type, name, enum_name)                                        \
    {                                                                                    \
        type bs_name;                                                                    \
        bs >> bs_name;                                                                   \
        name = (enum_name)bs_name;                                                       \
    }

#define END_LOADS }
// END LOAD MACROS
// -----------------------------------------------------------------------

#define NEW_OBJ(objType)                                                                 \
    static Hmx::Object *NewObject() { return new objType; }

#define REGISTER_OBJ_FACTORY(objType)                                                    \
    Hmx::Object::RegisterFactory(objType::StaticClassName(), objType::NewObject);

// TypeProps implementation
class TypeProps : public ObjRefOwner {
private:
    DataArray *mMap; // 0x4
    Hmx::Object *mOwner; // 0x8
    ObjPtrList<Hmx::Object> mObjects; // 0xc

    void ReplaceObject(DataNode &, Hmx::Object *, Hmx::Object *);
    void ReleaseObjects();
    void AddRefObjects();
    void ClearAll();

public:
    TypeProps(Hmx::Object *o)
        : mOwner(o), mMap(nullptr), mObjects(this, kObjListOwnerControl) {}
    virtual ~TypeProps() { ClearAll(); }
    virtual Hmx::Object *RefOwner() const { return mOwner; }
    virtual bool Replace(ObjRef *, Hmx::Object *);

    DataNode *KeyValue(Symbol, bool) const;
    int Size() const;
    void ClearKeyValue(Symbol);
    void SetKeyValue(Symbol, const DataNode &, bool);
    DataArray *GetArray(Symbol);
    void SetArrayValue(Symbol, int, const DataNode &);
    void RemoveArrayValue(Symbol, int);
    void InsertArrayValue(Symbol, int, const DataNode &);
    void Load(BinStreamRev &);
    TypeProps &operator=(const TypeProps &);
    void Save(BinStream &);
    DataArray *Map() const { return mMap; }
    bool HasProps() const { return mMap && mMap->Size() != 0; }

    MEM_OVERLOAD(TypeProps, 0x485);
};

typedef Hmx::Object *ObjectFunc(void);

// Hmx::Object implementation
namespace Hmx {

    class Object : public ObjRefOwner {
    private:
        void RemoveFromDir();

        DataNode OnIterateRefs(const DataArray *);
        DataNode OnSet(const DataArray *);
        DataNode OnPropertyAppend(const DataArray *);
        DataNode OnGetTypeList(const DataArray *);
        DataNode OnAddSink(DataArray *);
        DataNode OnRemoveSink(DataArray *);
        void ExportPropertyChange(DataArray *, Symbol);

        static std::map<Symbol, ObjectFunc *> sFactories;

    protected:
        ObjRef mRefs; // 0x4
        TypeProps *mTypeProps; // 0x10
    private: // these were marked private in RB2
        DataArray *mTypeDef; // 0x14
        String mNote; // 0x18
        const char *mName; // 0x20
        ObjectDir *mDir; // 0x24
        MsgSinks *mSinks; // 0x28
    protected:
        static Object *sDeleting;

        MsgSinks *GetOrAddSinks();
        DataNode OnGet(const DataArray *);
        void BroadcastPropertyChange(DataArray *);
        void BroadcastPropertyChange(Symbol);

    public:
        enum CopyType {
            kCopyDeep = 0,
            kCopyShallow = 1,
            kCopyFromMax = 2
        };

        enum SinkMode {
            /** "does a Handle to the sink, this gets all c handlers, type handling,
             * and exporting." */
            kHandle = 0,
            /** "just Exports to the sink, so no c or type handling" */
            kExport = 1,
            /** "just calls HandleType, good if know that particular thing is only
             * ever type handled." */
            kType = 2,
            /** "do type handling and exporting using Export, no C handling" */
            kExportType = 3,
        };

        Object();
        virtual ~Object();
        virtual Object *RefOwner() const { return const_cast<Object *>(this); }
        virtual bool Replace(ObjRef *, Hmx::Object *);
        OBJ_CLASSNAME(Object);
        OBJ_SET_TYPE(Object);
        virtual DataNode Handle(DataArray *, bool);
        virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
        virtual void InitObject();
        virtual void Save(BinStream &);
        virtual void Copy(const Hmx::Object *, CopyType);
        virtual void Load(BinStream &);
        virtual void PreSave(BinStream &) {}
        virtual void PostSave(BinStream &) {}
        virtual void Print() {}
        virtual void Export(DataArray *, bool);
        virtual void SetTypeDef(DataArray *);
        virtual DataArray *ObjectDef(Symbol);
        virtual void SetName(const char *, ObjectDir *);
        virtual ObjectDir *DataDir();
        virtual void PreLoad(BinStream &);
        virtual void PostLoad(BinStream &) {}
        virtual const char *FindPathName();

        Symbol Type() const {
            if (mTypeDef)
                return mTypeDef->Sym(0);
            else
                return Symbol();
        }
        // ObjRef *Refs() const { return (ObjRef *)&mRefs; }
        const ObjRef &Refs() const { return mRefs; }
        void SetNote(const char *note);
        DataArray *TypeDef() const { return mTypeDef; }
        ObjectDir *Dir() const { return mDir; }
        const char *Name() const { return mName; }
        const String &Note() const { return mNote; }
        const char *AllocHeapName() { return MemHeapName(MemFindAddrHeap(this)); }
        void AddRef(ObjRef *ref) { ref->AddRef(&mRefs); }
        void Release(ObjRef *ref) { ref->Release(&mRefs); }

        void ReplaceRefs(Hmx::Object *);
        void ReplaceRefsFrom(Hmx::Object *, Hmx::Object *);
        int RefCount() const;
        void RemovePropertySink(Hmx::Object *, DataArray *);
        bool HasPropertySink(Hmx::Object *, DataArray *);
        void RemoveSink(Hmx::Object *, Symbol = Symbol());
        void SaveType(BinStream &);
        void SaveRest(BinStream &);
        void ClearAllTypeProps();
        bool HasTypeProps() const;
        void AddSink(
            Hmx::Object *,
            Symbol = Symbol(),
            Symbol = Symbol(),
            SinkMode = kHandle,
            bool = true
        );
        void AddPropertySink(Hmx::Object *, DataArray *, Symbol);
        void MergeSinks(Hmx::Object *);
        DataNode PropertyArray(Symbol);
        int PropertySize(DataArray *);
        void InsertProperty(DataArray *, const DataNode &);
        void RemoveProperty(DataArray *);
        void PropertyClear(DataArray *);
        const DataNode *Property(DataArray *, bool) const;
        const DataNode *Property(Symbol, bool) const;
        DataNode HandleProperty(DataArray *, DataArray *, bool);
        DataNode HandleType(DataArray *);
        void ChainSource(Hmx::Object *, Hmx::Object *);
        void LoadType(BinStream &);
        void LoadRest(BinStream &);
        void SetProperty(Symbol, const DataNode &);
        void SetProperty(DataArray *, const DataNode &);

        NEW_OBJ(Hmx::Object);
        static Object *NewObject(Symbol);
        static bool RegisteredFactory(Symbol);
        static void RegisterFactory(Symbol, ObjectFunc *);

        /** Create a new Object derivative based on its entry in the factory list. */
        template <class T>
        static T *New() {
            T *obj = dynamic_cast<T *>(Hmx::Object::NewObject(T::StaticClassName()));
            if (!obj)
                MILO_FAIL("Couldn't instantiate class %s", T::StaticClassName());
            return obj;
        }
    };

}

extern bool gLoadingProxyFromDisk;

inline TextStream &operator<<(TextStream &ts, const Hmx::Object *obj) {
    if (obj)
        ts << obj->Name();
    else
        ts << "<null>";
    return ts;
}

// DataNodeObjTrack
class DataNodeObjTrack {
public:
    DataNodeObjTrack(const DataNode &node) : unk0(nullptr, nullptr) {
        unk14 = node.Evaluate();
        if (unk14.Type() == kDataObject) {
            unk0 = unk14.GetObj();
        }
    }
    DataNode Node() const {
        if (unk14.Type() == kDataObject) {
            return unk0.Ptr();
        } else
            return unk14;
    }
    DataNodeObjTrack &operator=(const DataNode &node) {
        unk14 = node.Evaluate();
        if (unk14.Type() == kDataObject) {
            unk0 = unk14.GetObj();
        }
        return *this;
    }
    // DataNodeObjTrack& operator=(const DataNodeObjTrack&);

protected:
    ObjPtr<Hmx::Object> unk0; // 0x0
    DataNode unk14; // 0x14
};

// ObjVector
template <class T>
class ObjVector : public std::vector<T> {
private:
    typedef typename std::vector<T> Base;
    Hmx::Object *mOwner;

public:
    ObjVector(Hmx::Object *o) : mOwner(o) {}

    Hmx::Object *Owner() { return mOwner; }

    void push_back() { resize(size() + 1); }

    void push_back(const T &t) {
        push_back();
        back() = t;
    }

    void resize(unsigned int size) { Base::resize(size, T(mOwner)); }

    void operator=(const ObjVector &vec) {
        if (this != &vec) {
            resize(vec.size());
            Base::operator=((Base &)vec);
        }
    }
};

template <class T>
BinStream &operator>>(BinStream &bs, ObjVector<T> &vec) {
    unsigned int length;
    bs >> length;
    vec.resize(length);

    for (ObjVector<T>::iterator it = vec.begin(); it != vec.end(); it++) {
        bs >> *it;
    }
    return bs;
}

// ObjList
template <class T>
class ObjList : public std::list<T> {
private:
    typedef typename std::list<T> Base;
    Hmx::Object *mOwner;

public:
    ObjList(Hmx::Object *o) : mOwner(o) {}

    Hmx::Object *Owner() { return mOwner; }

    void resize(unsigned int ul) { Base::resize(ul, T(mOwner)); }

    void push_front() { insert(begin(), T(mOwner)); }

    void push_front(const T &t) {
        push_front();
        front() = t;
    }

    void push_back() { resize(size() + 1); }

    void push_back(const T &t) {
        push_back();
        back() = t;
    }

    void operator=(const ObjList &oList) {
        if (this != &oList) {
            resize(oList.size());
            Base::operator=((Base &)oList);
        }
    }
};

template <class T>
BinStream &operator>>(BinStream &bs, ObjList<T> &oList) {
    unsigned int length;
    bs >> length;
    oList.resize(length);

    for (std::list<T>::iterator it = oList.begin(); it != oList.end(); ++it) {
        bs >> *it;
    }
    return bs;
}

// ObjectStage
class ObjectStage : public ObjPtr<Hmx::Object> {
public:
    ObjectStage() : ObjPtr<Hmx::Object>(sOwner) {}
    ObjectStage(Hmx::Object *o) : ObjPtr<Hmx::Object>(sOwner, o) {}
    virtual ~ObjectStage() {}

    static Hmx::Object *sOwner;
};

inline void
Interp(const ObjectStage &stage1, const ObjectStage &stage2, float f, Hmx::Object *&obj) {
    const ObjectStage &out = f < 1 ? stage1 : stage2;
    obj = out.Ptr();
}

BinStream &operator<<(BinStream &, const ObjectStage &);
BinStreamRev &operator>>(BinStreamRev &, ObjectStage &);

// ObjVersion
struct ObjVersion {
    ObjVersion(int i, Hmx::Object *o) : revs(i), obj(nullptr, o) {}
    ~ObjVersion() {}

    ObjPtr<Hmx::Object> obj;
    int revs;
};

#include "ObjPtr_p.h"
