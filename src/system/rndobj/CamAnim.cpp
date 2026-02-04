#include "rndobj/CamAnim.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"

#pragma region Hmx::Object

RndCamAnim::RndCamAnim() : mCam(this, 0), mKeysOwner(this, this) {}

RndCamAnim::~RndCamAnim() {}

bool RndCamAnim::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mKeysOwner == from) {
        if (mKeysOwner == this) {
            RndCamAnim *camTo = dynamic_cast<RndCamAnim *>(to);
            if (camTo) {
                mKeysOwner = camTo->KeysOwner();
            }
        } else {
            mKeysOwner = this;
        }
        return true;
    } else
        return Hmx::Object::Replace(from, to);
}

BEGIN_HANDLERS(RndCamAnim)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndCamAnim)
    SYNC_PROP(cam, mCam)
    SYNC_PROP(fov_keys, mFovKeys)
    SYNC_PROP(keys_owner, mKeysOwner)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndCamAnim)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mCam << mFovKeys << mKeysOwner;
END_SAVES

BEGIN_COPYS(RndCamAnim)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY(RndCamAnim)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mCam)
        if (ty == kCopyShallow || ty == kCopyFromMax && c->mKeysOwner != c) {
            mKeysOwner = c->mKeysOwner;
        } else {
            mKeysOwner = this;
            mFovKeys = c->mKeysOwner->mFovKeys;
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndCamAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    if (d.rev > 0) {
        Hmx::Object::Load(bs);
    }
    RndAnimatable::Load(bs);
    bs >> mCam;
    d >> mFovKeys >> mKeysOwner;
    if (d.rev < 2) {
        FOREACH (it, mFovKeys) {
            it->value = ConvertFov(it->value, 0.75);
        }
    }
    if (!mKeysOwner) {
        mKeysOwner = this;
    }
END_LOADS

#pragma endregion
#pragma region RndAnimatable

void RndCamAnim::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mCam) {
        if (!FovKeys().empty()) {
            float ref = mCam->YFov();
            FovKeys().AtFrame(frame, ref);
            if (blend != 1) {
                Interp(mCam->YFov(), ref, blend, ref);
            }
            mCam->SetFrustum(mCam->NearPlane(), mCam->FarPlane(), ref, 1.0f);
        }
    }
}

float RndCamAnim::EndFrame() { return FovKeys().LastFrame(); }

void RndCamAnim::SetKey(float frame) {
    if (mCam) {
        FovKeys().Add(mCam->YFov(), frame, true);
    }
}

#pragma endregion
