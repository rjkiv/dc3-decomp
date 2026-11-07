#include "char/CharGuitarString.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"

CharGuitarString::CharGuitarString()
    : mOpen(false), mNut(this), mBridge(this), mBend(this), mTarget(this) {}

CharGuitarString::~CharGuitarString() {}

BEGIN_HANDLERS(CharGuitarString)
    HANDLE_ACTION(set_open, mOpen = _msg->Int(2))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharGuitarString)
    SYNC_PROP(nut, mNut)
    SYNC_PROP(bridge, mBridge)
    SYNC_PROP(bend, mBend)
    SYNC_PROP(target, mTarget)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharGuitarString)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mNut;
    bs << mBridge;
    bs << mBend;
    bs << mTarget;
END_SAVES

BEGIN_COPYS(CharGuitarString)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharGuitarString)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mNut)
        COPY_MEMBER(mBridge)
        COPY_MEMBER(mBend)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(CharGuitarString)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mNut;
    d >> mBridge;
    d >> mBend;
    d >> mTarget;
END_LOADS

void CharGuitarString::Poll() {
    if (mNut && mBridge && mBend && mTarget) {
        Transform bendXfm = mBend->WorldXfm();
        const Vector3 &nutV = mNut->WorldXfm().v;
        const Vector3 &bridgeV = mBridge->WorldXfm().v;
        const Vector3 &targetV = mTarget->WorldXfm().v;
        Vector3 distNutToTarget;
        Subtract(targetV, nutV, distNutToTarget);
        Vector3 distNutToBridge;
        Subtract(bridgeV, nutV, distNutToBridge);
        float clamped = Clamp(
            0.0f,
            1.0f,
            Dot(distNutToTarget, distNutToBridge) / Dot(distNutToBridge, distNutToBridge)
        );
        if (mOpen) {
            clamped = 0;
        }
        Interp(nutV, bridgeV, clamped, bendXfm.v);
        mBend->SetWorldXfm(bendXfm);
    }
}

void CharGuitarString::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mNut);
    changedBy.push_back(mBridge);
    changedBy.push_back(mTarget);
    change.push_back(mBend);
}
