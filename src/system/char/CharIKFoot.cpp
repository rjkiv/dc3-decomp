#include "char/CharIKFoot.h"
#include "CharIKHand.h"
#include "char/Character.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Trans.h"

static const float sFSMFloat = 0.5f;

CharIKFoot::CharIKFoot() : mDummy(this), mState(0), mData(this), mDataIndex(0) {
    mDummy = Hmx::Object::New<RndTransformable>();
    mDummy->DirtyLocalXfm().Reset();
}

CharIKFoot::~CharIKFoot() { delete mDummy; }

BEGIN_HANDLERS(CharIKFoot)
    HANDLE_SUPERCLASS(CharIKHand)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKFoot)
    SYNC_PROP(data, mData)
    SYNC_PROP(data_index, mDataIndex)
    SYNC_SUPERCLASS(CharIKHand)
END_PROPSYNCS

BEGIN_SAVES(CharIKFoot)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(CharIKHand)
    bs << mData;
    bs << mDataIndex;
END_SAVES

BEGIN_COPYS(CharIKFoot)
    COPY_SUPERCLASS(CharIKHand)
    CREATE_COPY(CharIKFoot)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mData)
        COPY_MEMBER(mDataIndex)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(6, 0)

BEGIN_LOADS(CharIKFoot)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    LOAD_SUPERCLASS(CharIKHand)
    if (d.rev < 6) {
        Symbol s;
        d >> s;
    }
    if (d.rev < 5) {
        int i;
        if (d.rev > 1)
            d >> i;
        if (d.rev > 2)
            d >> i;
        if (d.rev > 3)
            d >> i;
    } else {
        d >> mData;
        d >> mDataIndex;
    }
END_LOADS

void CharIKFoot::Enter() {
    mState = 0;
    mLastDist = 0.0f;
}

void CharIKFoot::PollDeps(std::list<Hmx::Object *> &l1, std::list<Hmx::Object *> &l2) {
    CharIKHand::PollDeps(l1, l2);
}

void CharIKFoot::Poll() {
    if (mFinger && mHand && mData) {
        mTargets.clear();
        mTargets.push_back(IKTarget(mDummy, 0));
        DoFSM(Character::Current(), mDummy->DirtyLocalXfm());
        CharIKHand::Poll();
        mTargets.clear();
    }
}

void CharIKFoot::DoFSM(Character *c, Transform &tf) {
    mLastTrans = mFinger->WorldXfm();
    if (c && c->Teleported()) {
        mState = 0;
    }
    float secs = TheTaskMgr.DeltaSeconds();
    if (secs < 0) {
        secs = 0;
    }
    tf.m = mFinger->WorldXfm().m;
    tf.v.z = mFinger->WorldXfm().v.z;
    mFreezePos.z = tf.v.z;
    bool b7 = false;
    float vecIdx = mData->LocalXfm().v[mDataIndex];
    if (vecIdx >= 0.98f) {
        b7 = true;
    } else if (vecIdx > 0 && tf.v.z < (mState == 1 ? 0.6f : sFSMFloat)) {
        b7 = true;
    }
    if (mState == 0) {
        const Transform &fingerXfm = mFinger->WorldXfm();
        tf.v.x = fingerXfm.v.x;
        tf.v.y = fingerXfm.v.y;
        if (b7) {
            mFreezePos = tf.v;
            mState = 1;
        }
    }
    if (mState == 1) {
        if (!b7) {
            mState = 2;
            mLastDist = Distance(mFinger->WorldXfm().v, tf.v);
        } else {
            Vector3 v3c;
            Subtract(mFinger->WorldXfm().v, mFreezePos, v3c);
            float len = Length(v3c);
            if (len > 0.125f) {
                // Scale(v3c, 0.125f / len, v3c);
                v3c *= 0.125f / len;
            }
            Add(mFreezePos, v3c, tf.v);
            return;
        }
    }
    if (mState == 2) {
        Vector3 v48;
        Subtract(mFinger->WorldXfm().v, tf.v, v48);
        float len = Length(v48);
        mLastDist = Min(-(secs * 25.0f - mLastDist), len);
        if (mLastDist <= 0.0f)
            mState = 0;
        else
            v48 *= (len - mLastDist) / len;
        tf.v += v48;
        if (b7) {
            mFreezePos = tf.v;
            mState = 1;
        }
    }
}
