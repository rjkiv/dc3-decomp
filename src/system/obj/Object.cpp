#include "obj/Object.h"
#include "Dir.h"
#include "Msg.h"
#include "Object.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/Platform.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

bool gLoadingProxyFromDisk = false;
std::map<Symbol, ObjectFunc *> Hmx::Object::sFactories;
DataArrayPtr gPropPaths[8] = {
    DataArrayPtr(new DataArray(1)), DataArrayPtr(new DataArray(1)),
    DataArrayPtr(new DataArray(1)), DataArrayPtr(new DataArray(1)),
    DataArrayPtr(new DataArray(1)), DataArrayPtr(new DataArray(1)),
    DataArrayPtr(new DataArray(1)), DataArrayPtr(new DataArray(1))
};
MsgSinks gSinks(nullptr);

bool Hmx::Object::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (mSinks)
        return mSinks->Replace(ref, obj);
    else
        return false;
}

void Hmx::Object::RemovePropertySink(Hmx::Object *o, DataArray *a) {
    if (mSinks)
        mSinks->RemovePropertySink(o, a);
}

bool Hmx::Object::HasPropertySink(Hmx::Object *o, DataArray *a) {
    if (mSinks)
        return mSinks->HasPropertySink(o, a);
    else
        return false;
}

void Hmx::Object::RemoveSink(Hmx::Object *o, Symbol s) {
    if (mSinks)
        mSinks->RemoveSink(o, s);
}

ObjectDir *Hmx::Object::DataDir() { return mDir ? mDir : ObjectDir::sMainDir; }

Hmx::Object::Object()
    : mTypeProps(nullptr), mTypeDef(nullptr), mName(gNullStr), mDir(nullptr),
      mSinks(nullptr) {
    mRefs.Clear();
}

void Hmx::Object::ReplaceRefs(Hmx::Object *obj) {
    if (mRefs.begin() != mRefs.end()) {
        ObjRef other(mRefs);
        mRefs.Clear();
        other.ReplaceList(obj);
    }
}

void Hmx::Object::ReplaceRefsFrom(Hmx::Object *from, Hmx::Object *to) {
    MILO_ASSERT(from, 0xA6);
    ObjRef other;
    other.Clear();
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        if (it->RefOwner() == from) {
            it->Release(&other);
            other.AddRef(it);
        }
    }
    other.ReplaceList(to);
}

int Hmx::Object::RefCount() const { return mRefs.RefCount(); }

void Hmx::Object::RemoveFromDir() {
    if (mDir && mDir != sDeleting) {
        mDir->RemovingObject(this);
        ObjectDir::Entry *entry = mDir->FindEntry(mName, false);
        if (!entry || entry->obj != this) {
            MILO_FAIL("No entry for %s in %s", PathName(this), PathName(mDir));
        }
        entry->obj = nullptr;
    }
}

DataArray *GetNextPropPath() {
    for (int i = 0; i < 8; i++) {
        if (gPropPaths[i]->RefCount() == 1) {
            return gPropPaths[i];
        }
    }
    MILO_FAIL("Recursive SetProperty call count greater than %d!", 8);
    return nullptr;
}

__declspec(noinline) bool Hmx::Object::HasTypeProps() const {
    if (mTypeProps) {
        DataArray *tpMap = mTypeProps->Map();
        return tpMap && tpMap->Size() != 0;
    }
    return false;
}

MsgSinks *Hmx::Object::GetOrAddSinks() {
    if (!mSinks) {
        mSinks = new MsgSinks(this);
    }
    return mSinks;
}

void Hmx::Object::AddSink(Hmx::Object *o, Symbol s1, Symbol s2, SinkMode sm, bool b) {
    GetOrAddSinks()->AddSink(o, s1, s2, sm, b);
}

void Hmx::Object::AddPropertySink(Hmx::Object *o, DataArray *a, Symbol s) {
    GetOrAddSinks()->AddPropertySink(o, a, s);
}

void Hmx::Object::MergeSinks(Hmx::Object *o) {
    if (o && o->mSinks) {
        GetOrAddSinks()->MergeSinks(o);
    }
}

void Hmx::Object::SaveType(BinStream &bs) {
    bs << 2;
    bs << Type();
}

void Hmx::Object::SaveRest(BinStream &bs) {
    if (!mTypeProps)
        bs << (DataArray *)nullptr;
    else
        mTypeProps->Save(bs);

    if (mNote.empty() || (unsigned char)bs.GetPlatform() != kPlatformNone)
        bs << 0;
    else
        bs << mNote;
}

DataArray *Hmx::Object::ObjectDef(Symbol s) {
    Symbol sref;
    if (s == gNullStr) {
        s = ClassName();
        sref = "objects";
    } else
        sref = "objects";
    return SystemConfig(sref, s);
}

void Hmx::Object::LoadType(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    Symbol s;
    bs >> s;
    SetType(s);
    bsrev.PushRev(this);
}

const char *Hmx::Object::FindPathName() {
    const char *name = (mName && *mName) ? mName : ClassName().Str();

    class ObjectDir *dataDir = DataDir();
    if (dataDir) {
        if (dataDir->Loader()) {
            return MakeString(
                "%s (%s)",
                name,
                FileLocalize(dataDir->Loader()->LoaderFile().c_str(), nullptr)
            );
        } else if (!dataDir->ProxyFile().empty()) {
            return MakeString(
                "%s (%s)", name, FileLocalize(dataDir->ProxyFile().c_str(), nullptr)
            );
        } else if (*dataDir->GetPathName() != '\0') {
            return MakeString(
                "%s (%s)", name, FileLocalize(dataDir->GetPathName(), nullptr)
            );
        } else if (dataDir != this && dataDir->Name() && *dataDir->Name()) {
            return MakeString("%s/%s", dataDir->Name(), name);
        } else if (mDir && *mDir->GetPathName()) {
            return MakeString("%s (%s)", name, FileLocalize(mDir->GetPathName(), nullptr));
        }
    }
    return name;
}

Hmx::Object::~Object() {
    MILO_ASSERT_FMT(MainThread(), "Can't delete objects outside of the main thread");
    if (mTypeDef) {
        mTypeDef->Release();
        mTypeDef = nullptr;
    }
    ClearAllTypeProps();
    RemoveFromDir();
    RELEASE(mSinks);
    Hmx::Object *old = sDeleting;
    sDeleting = this;
    ReplaceRefs(nullptr);
    sDeleting = old;
    if (gDataThis == this) {
        gDataThis = nullptr;
    }
}

void Hmx::Object::SetName(const char *name, ObjectDir *dir) {
    RemoveFromDir();
    if (!name || *name == '\0') {
        mName = gNullStr;
        mDir = nullptr;
    } else {
        MILO_ASSERT(dir, 0xE7);
        mDir = dir;
        ObjectDir::Entry *entry = dir->FindEntry(name, true);
        if (entry->obj) {
            MILO_FAIL("%s already exists", name);
        }
        entry->obj = this;
        mName = entry->name;
        dir->AddedObject(this);
    }
}

void Hmx::Object::SetTypeDef(DataArray *def) {
    if (mTypeDef != def) {
        if (mTypeDef) {
            mTypeDef->Release();
            mTypeDef = nullptr;
        }
        ClearAllTypeProps();
        mTypeDef = def;
        if (mTypeDef) {
            mTypeDef->AddRef();
        }
    }
}

void Hmx::Object::ChainSource(Hmx::Object *source, Hmx::Object *o2) {
    MILO_ASSERT(source, 0x29D);
    if (!o2)
        o2 = this;
    if (mSinks) {
        source->GetOrAddSinks()->AddSink(this, gNullStr);
    }
}

void Hmx::Object::Save(BinStream &bs) {
    SaveType(bs);
    SaveRest(bs);
}

void Hmx::Object::InitObject() {
    static DataArray *objects = SystemConfig("objects");
    static Symbol init = "init";
    DataArray *def = ObjectDef(gNullStr)->FindArray(init, false);
    if (def) {
        def->ExecuteScript(1, this, nullptr, 1);
    }
}

DataNode Hmx::Object::HandleType(DataArray *msg) {
    Symbol t = msg->Sym(1);
    DataArray *handler = nullptr;
    if (mTypeDef) {
        handler = mTypeDef->FindArray(t, false);
    }
    if (handler) {
        MessageTimer timer(this, t);
        return handler->ExecuteScript(1, this, (const DataArray *)msg, 2);
    } else
        return DataNode(kDataUnhandled, 0);
}

DataNode Hmx::Object::OnIterateRefs(const DataArray *da) {
    DataNode *var = da->Var(2);
    DataNode node(*var);
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        *var = it->RefOwner();
        for (int i = 3; i < da->Size(); i++) {
            da->Command(i)->Execute(true);
        }
    }
    *var = node;
    return 0;
}

DataNode Hmx::Object::OnGetTypeList(const DataArray *a) {
    DataArray *def = ObjectDef(gNullStr);
    DataArrayPtr ptr;
    static Symbol allow_null_type = "allow_null_type";
    bool b6 = true;
    DataArray *nullArr = def->FindArray(allow_null_type, false);
    if (nullArr) {
        b6 = nullArr->ExecuteScript(1, this, nullptr, 1).Int();
    }
    if (b6) {
        ptr->Insert(ptr->Size(), gNullStr);
    }
    DataArray *typesArr = def->FindArray("types", false);
    if (typesArr) {
        for (int i = 1; i < typesArr->Size(); i++) {
            DataArray *curArr = typesArr->Array(i);
            DataArray *helpArr = curArr->FindArray("help", false);
            if (helpArr) {
                DataArray *newArr = new DataArray(2);
                newArr->Node(0) = curArr->Sym(0);
                newArr->Node(1) = DataNode(helpArr, kDataArray);
                ptr->Insert(ptr->Size(), DataNode(newArr, kDataArray));
                newArr->Release();
            } else {
                ptr->Insert(ptr->Size(), curArr->Sym(0));
            }
        }
    }
    return ptr;
}

DataNode Hmx::Object::OnAddSink(DataArray *a) {
    if (a->Size() > 3) {
        SinkMode mode = a->Size() > 4 ? (SinkMode)a->Int(4) : kHandle;
        bool chain = a->Size() > 5 ? a->Int(5) : true;
        DataArray *arr3 = a->Array(3);
        Hmx::Object *obj = a->Obj<Hmx::Object>(2);
        if (obj && arr3->Size() != 0) {
            for (int i = 0; i < arr3->Size(); i++) {
                DataNode eval = arr3->Evaluate(i);
                Symbol s6, s7;
                if (eval.Type() == kDataArray) {
                    s6 = eval.LiteralArray()->LiteralSym(1);
                    s7 = eval.LiteralArray()->LiteralSym(0);
                } else {
                    s7 = eval.LiteralSym();
                }
                GetOrAddSinks()->AddSink(obj, s7, s6, mode, chain);
            }
        } else {
            GetOrAddSinks()->AddSink(obj, Symbol(), Symbol(), mode, chain);
        }
    } else {
        Hmx::Object *obj = a->Obj<Hmx::Object>(2);
        GetOrAddSinks()->AddSink(obj, gNullStr);
    }
    return 0;
}

DataNode Hmx::Object::OnRemoveSink(DataArray *a) {
    if (a->Size() > 3) {
        Hmx::Object *obj = a->Obj<Hmx::Object>(2);
        for (int i = 3; i < a->Size(); i++) {
            Symbol s = a->Sym(i);
            if (mSinks)
                mSinks->RemoveSink(obj, s);
        }
    } else {
        Symbol s = Symbol();
        Hmx::Object *obj = a->Obj<Hmx::Object>(2);
        if (mSinks)
            mSinks->RemoveSink(obj, s);
    }
    return 0;
}

const DataNode *Hmx::Object::Property(DataArray *prop, bool fail) const {
    static DataNode n(0);
    // if prop was synced, return the prop node n
    if (const_cast<Hmx::Object *>(this)->SyncProperty(n, prop, 0, kPropGet))
        return &n;
    Symbol propKey = prop->Sym(0);

    if (mTypeProps) {
        // retrieve property val from typeprops array
        const DataNode *propValue = mTypeProps->KeyValue(propKey, false);
        if (!propValue) {
            if (mTypeDef) {
                DataArray *found = mTypeDef->FindArray(propKey, fail);
                if (found)
                    propValue = &found->Evaluate(1);
            }
        }
        if (propValue) {
            int cnt = prop->Size();
            if (cnt == 1)
                return propValue;
            else if (cnt == 2) {
                if (propValue->Type() == kDataArray) {
                    DataArray *ret = propValue->UncheckedArray();
                    return &ret->Node(prop->Int(1));
                }
            }
        }
    }
    if (fail) {
        MILO_FAIL("%s: property %s not found", PathName(this), PrintPropertyPath(prop));
    }
    return nullptr;
}

const DataNode *Hmx::Object::Property(Symbol prop, bool fail) const {
    static DataArrayPtr d(new DataArray(1));
    d->Node(0) = prop;
    return Property(d, fail);
}

DataNode Hmx::Object::HandleProperty(DataArray *prop, DataArray *a2, bool fail) {
    static DataNode n(a2, kDataArray);
    if (SyncProperty(n, prop, 0, kPropHandle)) {
        return n;
    }
    if (fail) {
        MILO_FAIL(
            "%s: property %s not found", PathName(this), prop ? prop->Sym(0) : "<none>"
        );
    }
    return 0;
}

void Hmx::Object::ExportPropertyChange(DataArray *a, Symbol s) {
    if (!s.Null()) {
        MILO_ASSERT(mSinks, 0x17F);
        static Message msg("blah", 0);
        msg.SetType(s);
        msg[0] = DataNode(a, kDataArray);
        Export(msg, true);
    }
}

DataNode Hmx::Object::PropertyArray(Symbol sym) {
    static DataArrayPtr d(new DataArray(1));
    d->Node(0) = sym;
    int size = PropertySize(d);
    DataArray *newArr = new DataArray(size);
    static DataArrayPtr path(new DataArray(2));
    path->Node(0) = sym;
    for (int i = 0; i < size; i++) {
        path->Node(1) = i;
        newArr->Node(i) = *Property(path, true);
    }
    DataNode ret = DataNode(newArr, kDataArray);
    newArr->Release();
    return ret;
}

void Hmx::Object::BroadcastPropertyChange(DataArray *a) {
    Symbol s;
    if (mSinks) {
        s = mSinks->GetPropSyncHandler(a);
    }
    ExportPropertyChange(a, s);
}

int Hmx::Object::PropertySize(DataArray *prop) {
    static DataNode n;
    if (SyncProperty(n, prop, 0, kPropSize)) {
        return n.Int();
    } else {
        MILO_ASSERT(prop->Size() == 1, 0x208);
        Symbol name = prop->Sym(0);
        const DataNode *a = mTypeProps->KeyValue(name, false);
        if (a == nullptr) {
            if (mTypeDef != nullptr) {
                a = &mTypeDef->FindArray(name)->Evaluate(1);
            } else
                MILO_FAIL("%s: property %s not found", PathName(this), name);
        }
        MILO_ASSERT(a->Type() == kDataArray, 0x21B);
        return a->UncheckedArray()->Size();
    }
    return 0;
}

void Hmx::Object::RemoveProperty(DataArray *prop) {
    static DataNode n;
    if (!SyncProperty(n, prop, 0, kPropRemove)) {
        MILO_ASSERT(prop->Size() == 2, 0x235);
        if (mTypeProps) {
            mTypeProps->RemoveArrayValue(prop->Sym(0), prop->Int(1));
        }
    }
}

void Hmx::Object::Export(DataArray *a, bool b) {
    if (b)
        HandleType(a);
    if (mSinks)
        mSinks->Export(a);
}

DataNode Hmx::Object::OnGet(const DataArray *a) {
    const DataNode &node = a->Evaluate(2);
    if (node.Type() == kDataSymbol) {
        const char *sym = node.UncheckedStr();
        const DataNode *prop = Property(STR_TO_SYM(sym), a->Size() < 4);
        if (prop)
            return *prop;
    } else {
        if (node.Type() != kDataArray) {
            String str;
            node.Print(str, true, 0);
            MILO_FAIL(
                "Data %s is not array or symbol (file %s, line %d)",
                str.c_str(),
                a->File(),
                a->Line()
            );
        }
        const DataNode *prop = Property(node.UncheckedArray(), a->Size() < 4);
        if (prop)
            return *prop;
    }
    return a->Node(3);
}

BEGIN_PROPSYNCS(Hmx::Object)
    SYNC_PROP_SET(name, mName, SetName(_val.Str(), mDir))
    SYNC_PROP_SET(type, Type(), SetType(_val.Sym()))
    SYNC_PROP(sinks, mSinks ? *mSinks : gSinks)
END_PROPSYNCS

Hmx::Object *Hmx::Object::NewObject(Symbol s) {
    std::map<Symbol, ObjectFunc *>::iterator it = sFactories.find(s);
    MILO_ASSERT_FMT(it != sFactories.end(), "Unknown class %s", s);
    return (it->second)();
}

bool Hmx::Object::RegisteredFactory(Symbol s) {
    return sFactories.find(s) != sFactories.end();
}

void Hmx::Object::BroadcastPropertyChange(Symbol s) {
    static DataArray *a = new DataArray(1);
    a->Node(0) = s;
    BroadcastPropertyChange(a);
}

void Hmx::Object::PropertyClear(DataArray *propArr) {
    int size = PropertySize(propArr);
    DataArray *cloned = propArr->Clone(true, false, 1);
    while (size-- != 0) {
        cloned->Node(cloned->Size() - 1) = size;
        RemoveProperty(cloned);
    }
    cloned->Release();
}

void Hmx::Object::RegisterFactory(Symbol s, ObjectFunc *func) { sFactories[s] = func; }

void Hmx::Object::SetProperty(DataArray *prop, const DataNode &val) {
    DataNode n;
    const DataNode *prop_n = nullptr;
    Symbol handler;
    if (mSinks) {
        handler = mSinks->GetPropSyncHandler(prop);
        if (!handler.Null()) {
            prop_n = Property(prop, false);
            if (prop_n)
                n = *prop_n;
        }
    }
    if (!SyncProperty((DataNode &)val, prop, 0, kPropSet)) {
        Symbol key = prop->Sym(0);
        if (!mTypeProps) {
            mTypeProps = new TypeProps(this);
        }
        if (prop->Size() == 1) {
            mTypeProps->SetKeyValue(key, val, true);
        } else {
            MILO_ASSERT(prop->Size() == 2, 0x1C4);
            mTypeProps->SetArrayValue(key, prop->Int(1), val);
        }
    } else {
        // val = Property(prop, true); // ???
    }

    if (prop_n && val.Equal(n, nullptr, false)) {
        handler = Symbol();
    }
    ExportPropertyChange(prop, handler);
}

void Hmx::Object::SetProperty(Symbol prop, const DataNode &val) {
    DataArray *path = GetNextPropPath();
    path->AddRef();
    path->Node(0) = prop;
    SetProperty(path, val);
    path->Release();
}

void Hmx::Object::InsertProperty(DataArray *prop, const DataNode &val) {
    if (!SyncProperty((DataNode &)val, prop, 0, kPropInsert)) {
        MILO_ASSERT(prop->Size() == 2, 0x240);
        if (!mTypeProps) {
            mTypeProps = new TypeProps(this);
        }
        mTypeProps->InsertArrayValue(prop->Sym(0), prop->Int(1), val);
    }
}

void Hmx::Object::Copy(const Hmx::Object *o, CopyType ty) {
    if (ty != kCopyFromMax) {
        mNote = o->mNote;
        if (ClassName() == o->ClassName()) {
            SetTypeDef(o->mTypeDef);
            if (o->HasTypeProps() && !mTypeProps) {
                mTypeProps = new TypeProps(this);
            } else if (!o->HasTypeProps()) {
                RELEASE(mTypeProps);
            }
            if (mTypeProps) {
                *mTypeProps = *o->mTypeProps;
            }
        } else if (o->mTypeDef || mTypeDef) {
            MILO_NOTIFY(
                "Can't copy type \"%s\" or type props of %s to %s, different classes %s and %s",
                o->Type(),
                mName,
                o->mName,
                o->ClassName(),
                ClassName()
            );
        }
    }
}

void Hmx::Object::LoadRest(BinStream &bs) {
    BinStreamRev bsrev(bs, bs.PopRev(this));
    if (!mTypeProps) {
        mTypeProps = new TypeProps(this);
    }
    mTypeProps->Load(bsrev);
    if (!mTypeProps->HasProps()) {
        RELEASE(mTypeProps);
    }
    if (bsrev.mRev > 0) {
        bsrev >> mNote;
    }
}

void Hmx::Object::Load(BinStream &bs) {
    LoadType(bs);
    LoadRest(bs);
}

DataNode Hmx::Object::OnSet(const DataArray *a) {
    MILO_ASSERT_FMT(
        a->Size() % 2 == 0,
        "Uneven number of properties (file %s, line %d)",
        a->File(),
        a->Line()
    );
    for (int i = 2; i < a->Size(); i += 2) {
        const DataNode &n = a->Evaluate(i);
        if (n.Type() == kDataSymbol) {
            const DataNode &eval = a->Evaluate(i + 1);
            const char *str = n.UncheckedStr();
            SetProperty(STR_TO_SYM(str), eval);
        } else {
            if (n.Type() != kDataArray) {
                String str;
                n.Print(str, true, 0);
                MILO_FAIL(
                    "Data %s is not array or symbol (file %s, line %d)",
                    str.c_str(),
                    a->File(),
                    a->Line()
                );
            }
            SetProperty(n.UncheckedArray(), a->Evaluate(i + 1));
        }
    }
    return 0;
}

DataNode Hmx::Object::OnPropertyAppend(const DataArray *da) {
    DataArray *arr = da->Array(2);
    int size = PropertySize(arr);
    DataArray *cloned = arr->Clone(true, false, 1);
    cloned->Node(cloned->Size() - 1) = size;
    InsertProperty(cloned, da->Evaluate(3));
    cloned->Release();
    return size;
}

BEGIN_HANDLERS(Hmx::Object)
    HANDLE(get, OnGet)
    HANDLE_EXPR(get_array, PropertyArray(_msg->Sym(2)))
    HANDLE_EXPR(size, PropertySize(_msg->Array(2)))
    HANDLE(set, OnSet)
    HANDLE_ACTION(insert, InsertProperty(_msg->Array(2), _msg->Evaluate(3)))
    HANDLE_ACTION(remove, RemoveProperty(_msg->Array(2)))
    HANDLE_ACTION(clear, PropertyClear(_msg->Array(2)))
    HANDLE(append, OnPropertyAppend)
    HANDLE_EXPR(has, Property(_msg->Array(2), false) != nullptr)
    HANDLE_EXPR(prop_handle, HandleProperty(_msg->Array(2), _msg, true))
    HANDLE_ACTION(copy, Copy(_msg->Obj<Hmx::Object>(2), (CopyType)_msg->Int(3)))
    HANDLE_EXPR(class_name, ClassName())
    HANDLE_EXPR(name, mName)
    HANDLE_EXPR(note, mNote)
    HANDLE_ACTION(set_note, SetNote(_msg->Str(2)))
    HANDLE(iterate_refs, OnIterateRefs)
    HANDLE_EXPR(dir, mDir)
    HANDLE_ACTION(
        set_name,
        SetName(_msg->Str(2), _msg->Size() > 3 ? _msg->Obj<ObjectDir>(3) : Dir())
    )
    HANDLE_ACTION(set_type, SetType(_msg->Sym(2)))
    HANDLE_EXPR(is_a, IsASubclass(ClassName(), _msg->Sym(2)))
    HANDLE_EXPR(get_type, Type())
    HANDLE_EXPR(get_heap, AllocHeapName())
    HANDLE(get_types_list, OnGetTypeList)
    HANDLE_ARRAY(mTypeDef)
    HANDLE(add_sink, OnAddSink)
    HANDLE(remove_sink, OnRemoveSink)
    Export(_msg, false);
END_HANDLERS
