#include "char/CharForeTwist.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "obj/Object.h"

CharForeTwist::CharForeTwist() : mHand(this), mTwist2(this), mOffset(0), mBias(0) {}

BEGIN_HANDLERS(CharForeTwist)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharForeTwist)
    SYNC_PROP(hand, mHand)
    SYNC_PROP(twist2, mTwist2)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(bias, mBias)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharForeTwist)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mOffset;
    bs << mHand;
    bs << mTwist2;
    bs << mBias;
END_SAVES

BEGIN_COPYS(CharForeTwist)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharForeTwist)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOffset)
        COPY_MEMBER(mHand)
        COPY_MEMBER(mTwist2)
        COPY_MEMBER(mBias)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BEGIN_LOADS(CharForeTwist)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mOffset;
    d >> mHand;
    d >> mTwist2;
    if (d.rev > 1 && d.rev < 3) {
        int dummy;
        d >> dummy;
    }
    if (d.rev > 3)
        d >> mBias;
END_LOADS

void CharForeTwist::Poll() {
    if (mHand && mTwist2 && mHand->TransParent() && mTwist2->TransParent()) {
        const Transform &parentxfm = mHand->TransParent()->WorldXfm();
        const Transform &handxfm = mHand->WorldXfm();
        Vector3 v98;
        Cross(parentxfm.m.y, handxfm.m.z, v98);
        float newbias = mBias * DEG2RAD;
        float clampxfms = Clamp(-1.0f, 1.0f, Dot(parentxfm.m.y, handxfm.m.z));
        float clamp98 = Clamp(-1.0f, 1.0f, Dot(parentxfm.m.x, v98));
        float sub =
            LimitAng(mOffset * DEG2RAD + atan2f(clamp98, clampxfms) + newbias) - newbias;
        if (!IsNaN(sub)) {
            Hmx::Matrix3 m58;
            MakeRotMatrixX(sub * 0.33333f, m58);
            RndTransformable *twistparent = mTwist2->TransParent();
            Transform tf88;
            tf88.v = parentxfm.v;
            Multiply(m58, parentxfm.m, tf88.m);
            twistparent->SetWorldXfm(tf88);
            Interp(
                tf88.v, handxfm.v, mTwist2->LocalXfm().v.x / mHand->LocalXfm().v.x, tf88.v
            );
            Multiply(m58, tf88.m, tf88.m);
            mTwist2->SetWorldXfm(tf88);
        }
    }
}

void CharForeTwist::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mHand);
    change.push_back(mTwist2);
    if (mTwist2)
        change.push_back(mTwist2->TransParent());
}
