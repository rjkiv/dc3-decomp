#include "Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

DataNode *TypeProps::KeyValue(Symbol key, bool fail) const {
    if (mMap) {
        for (int i = mMap->Size() - 2; i >= 0; i -= 2) {
            if (mMap->Node(i).UncheckedStr() == key.Str()) {
                return &mMap->Node(i + 1);
            }
        }
    }
    if (fail) {
        MILO_FAIL("Key %s not found", key);
    }
    return nullptr;
}

void GetSaveFlags(DataArray *arr, bool &proxy, bool &none) {
    static Symbol proxy_save = "proxy_save";
    static Symbol no_save = "no_save";
    if (arr) {
        for (int i = 2; i < arr->Size(); i++) {
            if (arr->Sym(i) == proxy_save)
                proxy = true;
            else if (arr->Sym(i) == no_save)
                none = true;
            else
                MILO_NOTIFY("Unknown type def attribute %s", arr->Sym(i));
        }
    }
}

int TypeProps::Size() const {
    if (mMap)
        return mMap->Size() / 2;
    else
        return 0;
}

void TypeProps::ClearKeyValue(Symbol key) {
    if (mMap) {
        for (int i = mMap->Size() - 2; i >= 0; i -= 2) {
            const char *cur_str = mMap->UncheckedStr(i);
            if (STR_TO_SYM(cur_str) == key) {
                DataNode &val = mMap->Node(i + 1);
                if (val.Type() == kDataObject) {
                    Hmx::Object *obj = val.UncheckedObj();
                    if (obj) {
                        mObjects.remove(obj);
                    }
                }
                mMap->Remove(i);
                mMap->Remove(i);
                if (mMap->Size() == 0 && mMap) {
                    mMap->Release();
                    mMap = nullptr;
                }
            }
        }
    }
}

void TypeProps::SetKeyValue(Symbol key, const DataNode &n, bool b) {
    if (b && n.Type() == kDataObject) {
        Hmx::Object *obj = n.UncheckedObj();
        if (obj) {
            mObjects.push_back(obj);
        }
    }
    if (!mMap) {
        mMap = new DataArray(2);
        mMap->Node(0) = key;
        mMap->Node(1) = n;
    } else {
        int size = mMap->Size();
        for (int i = size - 2; i >= 0; i -= 2) {
            if (mMap->UncheckedInt(i) == key) {
                DataNode &n = mMap->Node(i + 1);
                if (n.Type() == kDataObject) {
                    Hmx::Object *obj = n.UncheckedObj();
                    if (obj) {
                        mObjects.remove(obj);
                    }
                }
            }
        }
        mMap->Resize(size + 2);
        mMap->Node(size) = key;
        mMap->Node(size + 1) = n;
    }
}

void TypeProps::ReplaceObject(DataNode &n, Hmx::Object *o1, Hmx::Object *o2) {
    Hmx::Object *from = n.UncheckedObj();
    if (from == o1) {
        mObjects.remove(from);
        n = o2;
        if (o2) {
            mObjects.push_back(o2);
        }
    }
}

bool TypeProps::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (!mMap)
        return false;
    Hmx::Object *refObj = ref->GetObj();
    for (int i = mMap->Size() - 1; i > 0; i -= 2) {
        DataNode &node = mMap->Node(i);
        if (node.Type() == kDataObject) {
            ReplaceObject(node, refObj, obj);
        } else if (node.Type() == kDataArray) {
            DataArray *inner = node.UncheckedArray();
            for (int j = inner->Size() - 1; j >= 0; j--) {
                DataNode &node2 = inner->Node(j);
                if (node2.Type() == kDataObject) {
                    ReplaceObject(node2, refObj, obj);
                }
            }
        }
    }
    return true;
}

void TypeProps::ReleaseObjects() {
    if (mMap)
        mObjects.clear();
}

void TypeProps::AddRefObjects() {
    if (mMap) {
        for (int i = mMap->Size() - 1; i > 0; i -= 2) {
            DataNode &node = mMap->Node(i);
            if (node.Type() == kDataObject) {
                Hmx::Object *obj = node.UncheckedObj();
                if (obj)
                    mObjects.push_back(obj);
            } else if (node.Type() == kDataArray) {
                DataArray *inner = node.UncheckedArray();
                for (int j = inner->Size() - 1; j >= 0; j--) {
                    DataNode &node2 = inner->Node(j);
                    if (node2.Type() == kDataObject) {
                        Hmx::Object *obj = node.UncheckedObj();
                        if (obj)
                            mObjects.push_back(obj);
                    }
                }
            }
        }
    }
}

void TypeProps::ClearAll() {
    ReleaseObjects();
    if (mMap) {
        mMap->Release();
        mMap = nullptr;
    }
}

DataArray *TypeProps::GetArray(Symbol key) {
    DataArray *typeDef = mOwner->TypeDef();
    DataNode *n = KeyValue(key, false);
    DataArray *ret;
    if (!n) {
        MILO_ASSERT(typeDef, 0x18);
        DataArray *keyArray = typeDef->FindArray(key);
        DataArray *cloned = keyArray->Array(1)->Clone(true, false, 0);
        SetKeyValue(key, DataNode(cloned, kDataArray), true);
        ret = cloned;
        cloned->Release();
    } else {
        MILO_ASSERT(n->Type() == kDataArray, 0x1F);
        ret = n->UncheckedArray();
    }
    return ret;
}

void TypeProps::SetArrayValue(Symbol key, int i, const DataNode &value) {
    DataNode &n = GetArray(key)->Node(i);
    if (n.Type() == kDataObject) {
        Hmx::Object *obj = n.UncheckedObj();
        if (obj) {
            mObjects.remove(obj);
        }
    }
    n = value;
    if (n.Type() == kDataObject) {
        Hmx::Object *obj = n.UncheckedObj();
        if (obj) {
            mObjects.push_back(obj);
        }
    }
}

void TypeProps::RemoveArrayValue(Symbol key, int i) {
    DataArray *a = GetArray(key);
    DataNode &n = a->Node(i);
    if (n.Type() == kDataObject) {
        Hmx::Object *obj = n.UncheckedObj();
        if (obj) {
            mObjects.remove(obj);
        }
    }
    a->Remove(i);
}

void TypeProps::InsertArrayValue(Symbol key, int i, const DataNode &value) {
    GetArray(key)->Insert(i, value);
    if (value.Type() == kDataObject) {
        Hmx::Object *obj = value.UncheckedObj();
        if (obj) {
            mObjects.push_back(obj);
        }
    }
}

void TypeProps::Load(BinStreamRev &bs) {
    bool rev = bs.mRev < 2;
    ReleaseObjects();
    DataArray *def = RefOwner()->TypeDef();
    Hmx::Object *theThis = nullptr;
    if (def)
        theThis = DataSetThis(mOwner);
    if (mMap && gLoadingProxyFromDisk) {
        DataArray *oldMap = mMap;
        bs >> mMap;
        int oldMapSize = oldMap->Size();
        for (int i = 0; i < oldMapSize; i += 2) {
            Symbol val = oldMap->Sym(i);
            if (rev) {
                bool proxy = false;
                bool none = false;
                GetSaveFlags(def->FindArray(val, false), proxy, none);
                if (proxy || none)
                    continue;
            }
            SetKeyValue(val, oldMap->Node(i + 1), false);
        }
        oldMap->Release();
    } else {
        if (mMap) {
            mMap->Release();
            mMap = nullptr;
        }
        bs >> mMap;
    }
    if (def) {
        if (mMap && TheLoadMgr.EditMode()) {
            for (int i = 0; mMap && i < mMap->Size(); i += 2) {
                DataArray *found = def->FindArray(mMap->Sym(i), false);
                if (found && found->Type(1) != kDataCommand) {
                    if (!found->Node(1).CompatibleType(mMap->Type(i + 1))) {
                        MILO_LOG(
                            "%s: type based property \"%s\" is outdated, will clear on save\n",
                            PathName(mOwner),
                            mMap->Sym(i)
                        );
                    }
                }
            }
        }
        DataSetThis(theThis);
        AddRefObjects();
    }
}

TypeProps &TypeProps::operator=(const TypeProps &t) {
    ClearAll();
    if (t.mMap) {
        mMap = t.mMap->Clone(true, false, 0);
    }
    AddRefObjects();
    return *this;
}

void TypeProps::Save(BinStream &bs) {
    Hmx::Object *ref = RefOwner();
    if (mMap) {
        if (TheLoadMgr.EditMode()) {
            DataArray *def = ref->TypeDef();
            if (def) {
                for (int i = 0; mMap && i < mMap->Size(); i += 2) {
                    DataArray *arr = def->FindArray(mMap->Sym(i), false);
                    if (arr && arr->Type(1) != kDataCommand) {
                        if (arr->Node(1).CompatibleType(mMap->Type(i + 1)))
                            continue;
                        ClearKeyValue(mMap->Sym(i));
                        if (mMap)
                            i -= 2;
                        else
                            break;
                    }
                }
            }
        }
        if (mMap && ref->DataDir() == ref && ref->Dir() != ref || gLoadingProxyFromDisk) {
            DataArray *def = ref->TypeDef();
            std::list<Symbol> classnames;
            ObjectDir *refDir = dynamic_cast<ObjectDir *>(ref);
            if (refDir) {
                for (ObjDirItr<ObjectDir> it(refDir, false); it != nullptr; ++it) {
                    DataArrayPtr props = it->GetExposedProperties();
                    for (int i = 0; i < props->Size(); i++) {
                        classnames.push_back(props->Array(i)->Sym(0));
                    }
                }
            }

            DataArray *arrToWrite = nullptr;
            int keyIdx = 0;
            for (int i = 0; i < mMap->Size(); i += 2) {
                Symbol key = mMap->Sym(i);
                DataArray *keyArr = def->FindArray(key, false);
                if (keyArr) {
                    bool saveProxy = false;
                    bool saveNone = false;
                    GetSaveFlags(keyArr, saveProxy, saveNone);
                    if (!saveNone && saveProxy != gLoadingProxyFromDisk) {
                        if (!arrToWrite) {
                            arrToWrite = new DataArray(mMap->Size());
                        }
                        arrToWrite->Node(keyIdx) = key;
                        arrToWrite->Node(keyIdx + 1) = mMap->Node(i + 1);
                        keyIdx += 2;
                    }
                }
            }
            if (arrToWrite) {
                // resize arrToWrite to however many properties were actually inserted
                arrToWrite->Resize(keyIdx);
                bs << arrToWrite;
                arrToWrite->Release();
            } else
                bs << arrToWrite;
            return;
        }
    }
    std::list<Symbol> listb0;
    std::list<Hmx::Object *> lista8;
    for (int i = 0; i < mMap->Size(); i += 2) {
        Symbol key = mMap->Sym(i);
        DataNode &n = mMap->Node(i + 1);
        if (n.Type() == kDataObject) {
            Hmx::Object *obj = n.GetObj();
            if (obj) {
                ObjectDir *objDir = obj->Dir();
                if (objDir) {
                    if (objDir->ClassName() == "EditorDir") {
                        listb0.push_back(key);
                        lista8.push_back(obj);
                        mMap->Remove(i);
                        mMap->Remove(i);
                    }
                }
            }
        }
    }
    bs << mMap;
    std::list<Hmx::Object *>::const_iterator oit = lista8.begin();
    for (std::list<Symbol>::const_iterator sit = listb0.begin(); sit != listb0.end();
         ++sit, ++oit) {
        mMap->Insert(0, *oit);
        mMap->Insert(0, *sit);
    }
}
