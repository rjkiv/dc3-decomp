#include "char/CharWeightable.h"
#include "obj/Object.h"

CharWeightable::CharWeightable() : mWeight(1), mWeightOwner(this, this) {}

bool CharWeightable::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (&mWeightOwner == ref) {
        if (!mWeightOwner.SetObj(obj)) {
            mWeightOwner = this;
        }
        return true;
    } else {
        return Hmx::Object::Replace(ref, obj);
    }
}

BEGIN_HANDLERS(CharWeightable)
    HANDLE_VIRTUAL_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharWeightable)
    SYNC_PROP_SET(weight, mWeight, SetWeight(_val.Float()))
    SYNC_PROP_SET(
        weight_owner, mWeightOwner.Ptr(), SetWeightOwner(_val.Obj<CharWeightable>())
    )
    SYNC_VIRTUAL_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharWeightable)
    SAVE_REVS(3, 0)
    SAVE_VIRTUAL_SUPERCLASS(Hmx::Object)
    bs << mWeight;
    bs << mWeightOwner;
END_SAVES

BEGIN_COPYS(CharWeightable)
    COPY_VIRTUAL_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharWeightable)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyShallow) {
            SetWeightOwner(c->mWeightOwner);
        } else {
            SetWeightOwner(this);
            mWeight = c->mWeightOwner->mWeight;
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharWeightable)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    if (gRev > 2) {
        LOAD_VIRTUAL_SUPERCLASS(Hmx::Object)
    }
    bs >> mWeight;
    if (gRev > 1) {
        bs >> mWeightOwner;
    }
END_LOADS
