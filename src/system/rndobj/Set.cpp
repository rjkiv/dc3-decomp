#include "rndobj/Set.h"
#include "obj/Dir.h"

#include "utl/BinStream.h"

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

void RndSet::Save(BinStream &bs) {
    bs << 0;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mObjects;
}

BEGIN_COPYS(RndSet)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndSet)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mObjects)
    END_COPYING_MEMBERS
END_COPYS

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

RndSet::RndSet() : mObjects(this, kObjListNoNull) {}
