#pragma once
#include "obj/ObjPtr_p.h"
#include "obj/ObjRef.h"
#include "obj/Data.h" /* IWYU pragma: keep */
#include "obj/PropSync_p.h" /* IWYU pragma: keep */
#include "obj/MessageTimer.h" /* IWYU pragma: keep */
#include "utl/BinStream.h" /* IWYU pragma: keep */
#include "utl/MemMgr.h" /* IWYU pragma: keep */
#include "utl/Symbol.h" /* IWYU pragma: keep */
#include <map> /* IWYU pragma: keep */

class MsgSinks;
namespace Hmx {
    class Object;
}

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

#define HANDLE_ARRAY(array)                                                              \
    {                                                                                    \
        /* this needs to be placed up here to match Hmx::Object::Handle */               \
        DataArray *found;                                                                \
        if (array && (found = array->FindArray(sym, false))) {                           \
            _HANDLE_CHECKED(found->ExecuteScript(1, this, _msg, 2))                      \
        }                                                                                \
    }

#define HANDLE_FORWARD(func) _HANDLE_CHECKED(func(_msg, false))

#define HANDLE_SUPERCLASS(parent) HANDLE_FORWARD(parent::Handle)

#define END_HANDLERS                                                                     \
    if (_warn)                                                                           \
        MILO_NOTIFY("%s unhandled msg: %s", PathName(this), sym);                        \
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
#define SAVE_SUPERCLASS(parent) parent::Save(bs);
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
// END LOAD MACROS -----------------------------------------------------------------------

#define NEW_OBJ(objType)                                                                 \
    static Hmx::Object *NewObject() { return new objType; }

#define REGISTER_OBJ_FACTORY(objType)                                                    \
    Hmx::Object::RegisterFactory(objType::StaticClassName(), objType::NewObject);

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
            /** "does a Handle to the sink, this gets all c handlers, type handling, and
             * exporting." */
            kHandle = 0,
            /** "just Exports to the sink, so no c or type handling" */
            kExport = 1,
            /** "just calls HandleType, good if know that particular thing is only ever
             * type handled." */
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
        void SetNote(const char *note) { mNote = note; }
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
