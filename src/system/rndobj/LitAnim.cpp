#include "rndobj/LitAnim.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"

#pragma region Hmx::Object

RndLightAnim::RndLightAnim() : mLight(this), mKeysOwner(this, this) {}

bool RndLightAnim::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mLight == from) {
        if (mKeysOwner != this) {
            mKeysOwner = this;
        } else {
            RndLightAnim *litTo = dynamic_cast<RndLightAnim *>(to);
            if (litTo) {
                mKeysOwner = litTo;
            } else
                mKeysOwner = this;
        }
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndLightAnim)
    HANDLE(copy_keys, OnCopyKeys)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndLightAnim)
    SYNC_PROP(light, mLight)
    SYNC_PROP(color_keys, mColorKeys)
    SYNC_PROP(keys_owner, mKeysOwner)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndLightAnim)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mLight << mColorKeys << mKeysOwner;
END_SAVES

BEGIN_COPYS(RndLightAnim)
    CREATE_COPY_AS(RndLightAnim, l);
    MILO_ASSERT(l, 0x72);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(l, mLight)
    if (ty == kCopyShallow || ty == kCopyFromMax && l->mKeysOwner != l) {
        mKeysOwner = l->mKeysOwner;
    } else {
        mKeysOwner = this;
        mColorKeys = l->mKeysOwner->mColorKeys;
    }
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndLightAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    if (d.rev > 1) {
        Hmx::Object::Load(bs);
    }
    RndAnimatable::Load(bs);
    bs >> mLight;
    if (d.rev < 1) {
        Keys<Hmx::Color, Hmx::Color> keys;
        d >> keys;
    }
    d >> mColorKeys;
    if (d.rev < 1) {
        Keys<Hmx::Color, Hmx::Color> keys;
        d >> keys;
    }
    d >> mKeysOwner;
    if (!mKeysOwner) {
        mKeysOwner = this;
    }
END_LOADS

#pragma endregion
#pragma region RndAnimatable

void RndLightAnim::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mLight) {
        if (!ColorKeys().empty()) {
            Hmx::Color ref;
            ColorKeys().AtFrame(frame, ref);
            if (blend != 1.0f) {
                Interp(mLight->GetColor(), ref, blend, ref);
            }
            mLight->SetColor(ref);
        }
    }
}

float RndLightAnim::EndFrame() { return ColorKeys().LastFrame(); }

void RndLightAnim::SetKey(float frame) {
    if (mLight) {
        ColorKeys().Add(mLight->GetColor(), frame, true);
    }
}

#pragma endregion
#pragma region RndLightAnim

void RndLightAnim::SetKeysOwner(RndLightAnim *o) {
    MILO_ASSERT(o, 0x27);
    mKeysOwner = o;
}

#pragma endregion
#pragma region Handlers

DataNode RndLightAnim::OnCopyKeys(DataArray *a) {
    SetKeysOwner(this);
    mColorKeys = a->Obj<RndLightAnim>(2)->ColorKeys();
    float f = a->Float(3);
    FOREACH (it, mColorKeys) {
        Multiply(it->value, f, it->value);
    }
    return 0;
}

#pragma endregion
