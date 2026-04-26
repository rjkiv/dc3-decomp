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
            if ((int)mMap->Node(i).UncheckedStr() == (int)key.Str()) {
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
            if (mMap->UncheckedInt(i) == key) {
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
                return;
            }
        }
    }
}

void TypeProps::SetKeyValue(Symbol key, const DataNode &value, bool b) {
    if (b && value.Type() == kDataObject) {
        Hmx::Object *obj = value.UncheckedObj();
        if (obj) {
            mObjects.push_back(obj);
        }
    }
    if (!mMap) {
        mMap = new DataArray(2);
        mMap->Node(0) = key;
        mMap->Node(1) = value;
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
                n = value;
                return;
            }
        }
        mMap->Resize(size + 2);
        mMap->Node(size) = key;
        mMap->Node(size + 1) = value;
    }
}

void TypeProps::ReplaceObject(DataNode &n, Hmx::Object *from, Hmx::Object *to) {
    Hmx::Object *fromObj = n.UncheckedObj();
    if (fromObj == from) {
        mObjects.remove(fromObj);
        n = to;
        if (to) {
            mObjects.push_back(to);
        }
    }
}

bool TypeProps::Replace(ObjRef *from, Hmx::Object *to) {
    if (!mMap)
        return false;
    Hmx::Object *fromObj = from->GetObj();
    for (int i = mMap->Size() - 1; i > 0; i -= 2) {
        DataNode &node = mMap->Node(i);
        if (node.Type() == kDataObject) {
            ReplaceObject(node, fromObj, to);
        } else if (node.Type() == kDataArray) {
            DataArray *inner = node.UncheckedArray();
            for (int j = inner->Size() - 1; j >= 0; j--) {
                DataNode &node2 = inner->Node(j);
                if (node2.Type() == kDataObject) {
                    ReplaceObject(node2, fromObj, to);
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
                if (obj) {
                    mObjects.push_back(obj);
                }
            } else if (node.Type() == kDataArray) {
                DataArray *inner = node.UncheckedArray();
                for (int j = inner->Size() - 1; j >= 0; j--) {
                    DataNode &node2 = inner->Node(j);
                    if (node2.Type() == kDataObject) {
                        Hmx::Object *obj = node2.UncheckedObj();
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

DataArray *TypeProps::GetArray(Symbol prop) {
    DataArray *typeDef = mOwner->TypeDef();
    DataNode *n = KeyValue(prop, false);
    DataArray *ret;
    if (!n) {
        MILO_ASSERT(typeDef, 0x18);
        DataArray *keyArray = typeDef->FindArray(prop);
        DataArray *cloned = keyArray->Array(1)->Clone(true, false, 0);
        SetKeyValue(prop, cloned, true);
        ret = cloned;
        cloned->Release();
    } else {
        MILO_ASSERT(n->Type() == kDataArray, 0x1F);
        ret = n->UncheckedArray();
    }
    return ret;
}

void TypeProps::SetArrayValue(Symbol prop, int i, const DataNode &value) {
    DataNode &n = GetArray(prop)->Node(i);
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

void TypeProps::RemoveArrayValue(Symbol prop, int i) {
    DataArray *a = GetArray(prop);
    DataNode &n = a->Node(i);
    if (n.Type() == kDataObject) {
        Hmx::Object *obj = n.UncheckedObj();
        if (obj) {
            mObjects.remove(obj);
        }
    }
    a->Remove(i);
}

void TypeProps::InsertArrayValue(Symbol prop, int i, const DataNode &value) {
    DataArray *array = GetArray(prop);
    array->Insert(i, value);
    if (value.Type() == kDataObject) {
        Hmx::Object *obj = value.UncheckedObj();
        if (obj) {
            mObjects.push_back(obj);
        }
    }
}

void TypeProps::Load(BinStreamRev &d) {
    bool rev = d.rev < 2;
    ReleaseObjects();
    DataArray *def = RefOwner()->TypeDef();
    Hmx::Object *theThis = nullptr;
    if (def)
        theThis = DataSetThis(mOwner);
    if (mMap && gLoadingProxyFromDisk) {
        DataArray *oldMap = mMap;
        d >> mMap;
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
        d >> mMap;
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
    Hmx::Object *owner = RefOwner();
    if (mMap) {
        if (TheLoadMgr.EditMode()) {
            DataArray *typeDef = owner->TypeDef();
            if (typeDef) {
                for (int i = 0; mMap && i < mMap->Size();) {
                    DataArray *arr = typeDef->FindArray(mMap->Sym(i), false);
                    if (arr && arr->Type(1) != kDataCommand
                        && !arr->Node(1).CompatibleType(mMap->Type(i + 1))) {
                        ClearKeyValue(mMap->Sym(i));
                    } else {
                        i += 2;
                    }
                }
            }
        }
        std::list<Symbol> keys;
        std::list<Hmx::Object *> values;
        if (mMap) {
            for (int j = 0; j < mMap->Size();) {
                Symbol key = mMap->Sym(j);
                DataNode &value = mMap->Node(j + 1);
                if (value.Type() == kDataObject) {
                    Hmx::Object *valObj = value.GetObj();
                    if (valObj) {
                        ObjectDir *valObjDir = valObj->Dir();
                        if (valObjDir) {
                            if (valObjDir->ClassName() == "EditorDir") {
                                keys.push_back(key);
                                values.push_back(valObj);
                                mMap->Remove(j);
                                mMap->Remove(j);
                            } else {
                                j += 2;
                            }
                        }
                    }
                } else {
                    j += 2;
                }
            }
        }
        if (mMap && owner->DataDir() == owner && owner->Dir() != owner
            || gLoadingProxyFromDisk) {
            DataArray *typeDef = owner->TypeDef();
            std::list<Symbol> classnames;
            ObjectDir *ownerDir = dynamic_cast<ObjectDir *>(owner);
            if (ownerDir) {
                for (ObjDirItr<ObjectDir> it(ownerDir, false); it != nullptr; ++it) {
                    DataArrayPtr props = it->GetExposedProperties();
                    for (int i = 0; i < props->Size(); i++) {
                        classnames.push_back(props->Array(i)->Sym(0));
                    }
                }
            }
            if (mMap->Size() > 0) {
                DataArray *arrToWrite = nullptr;
                int keyIdx = 0;
                for (int i = 0; i < mMap->Size(); i += 2) {
                    Symbol key = mMap->Sym(i);
                    if (typeDef) {
                        arrToWrite = typeDef->FindArray(key, false);
                    }
                    bool isProxy = false;
                    bool none = false;
                    bool proxy = false;
                    if (arrToWrite) {
                        GetSaveFlags(arrToWrite, proxy, none);
                        isProxy = proxy;
                    }
                    if (!none && !isProxy && classnames.empty()) {
                        // something
                    }
                    if (!none && isProxy != gLoadingProxyFromDisk) {
                        if (!arrToWrite) {
                            arrToWrite = new DataArray(mMap->Size());
                        }
                        arrToWrite->Node(keyIdx) = key;
                        arrToWrite->Node(keyIdx + 1) = mMap->Node(i + 1);
                        keyIdx += 2;
                    }
                }
                if (arrToWrite && keyIdx > 0) {
                    arrToWrite->Resize(keyIdx);
                    bs << arrToWrite;
                    arrToWrite->Release();
                } else {
                    bs << arrToWrite;
                }
            } else {
                bs << mMap;
            }
            return;
        }

        bs << mMap;
        auto keysIt = keys.begin();
        auto valsIt = values.begin();
        for (; keysIt != keys.end(); ++keysIt, ++valsIt) {
            mMap->Insert(0, *keysIt);
            mMap->Insert(0, *valsIt);
        }
    }
}
