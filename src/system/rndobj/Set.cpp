#include "rndobj/Set.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "utl/BinStream.h"

RndSet::RndSet() : mObjects(this) {}

BEGIN_HANDLERS(RndSet)
    HANDLE(allowed_objects, OnAllowedObjects)
    HANDLE_SUPERCLASS(Hmx::Object)
    for (ObjPtrList<Hmx::Object>::iterator it = mObjects.begin(); it != mObjects.end();
         ++it) {
        (*it)->Handle(_msg, true);
    }
END_HANDLERS

BEGIN_PROPSYNCS(RndSet)
    SYNC_PROP(objects, mObjects)
    SYNC_SUPERCLASS(Hmx::Object)
    if (_op == kPropSet) {
        for (ObjPtrList<Hmx::Object>::iterator it = mObjects.begin();
             it != mObjects.end();
             ++it) {
            (*it)->SetProperty(_prop, _val);
        }
    }
END_PROPSYNCS

BEGIN_SAVES(RndSet)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mObjects;
END_SAVES

BEGIN_COPYS(RndSet)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndSet)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mObjects)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndSet)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mObjects;
END_LOADS

void RndSet::SetTypeDef(DataArray *def) {
    Hmx::Object::SetTypeDef(def);
    if (def) {
        DataArray *cfg = TypeDef()->FindArray("editor");
        mProps.resize(cfg->Size() - 1);
        for (int i = 1; i < cfg->Size(); i++) {
            const DataArray *thisArr = cfg->Array(i);
            DataNode &thisNode = thisArr->Node(1);
            if (thisNode.Type() != kDataSymbol) {
                MILO_NOTIFY("%s not top-level property in %s", thisArr->Sym(0), Name());
            }
            mProps[i - 1] = thisArr->Sym(0);
        }
        ObjPtrList<Hmx::Object>::iterator it = mObjects.begin();
        while (it != mObjects.end()) {
            if (!AllowedObject(*it)) {
                MILO_NOTIFY("%s not allowed in set", (*it)->Name());
                it = mObjects.erase(it);
            } else
                ++it;
        }
    } else {
        mProps.clear();
    }
}

bool RndSet::AllowedObject(Hmx::Object *o) {
    if (!o || o == this)
        return false;
    else {
        for (int i = 0; i < mProps.size(); i++) {
            if (o->Property(mProps[i], false) == nullptr) {
                return false;
            }
        }
        return true;
    }
}

DataNode RndSet::OnAllowedObjects(DataArray *) {
    std::list<Hmx::Object *> objList;
    for (ObjDirItr<Hmx::Object> it(Dir(), true); it != 0; ++it) {
        if (AllowedObject(it))
            objList.push_back(it);
    }
    DataArrayPtr ptr(new DataArray(objList.size()));
    int count = 0;
    for (std::list<Hmx::Object *>::iterator it = objList.begin(); it != objList.end();
         it++) {
        ptr->Node(count++) = *it;
    }
    return ptr;
}
