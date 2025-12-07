#include "char/CharIKRod.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

CharIKRod::CharIKRod()
    : mLeftEnd(this), mRightEnd(this), mDestPos(0.5), mSideAxis(this), mVertical(0),
      mDest(this) {}

CharIKRod::~CharIKRod() {}

BEGIN_HANDLERS(CharIKRod)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKRod)
    SYNC_PROP_MODIFY(left_end, mLeftEnd, SyncBones())
    SYNC_PROP_MODIFY(right_end, mRightEnd, SyncBones())
    SYNC_PROP_MODIFY(dest_pos, mDestPos, SyncBones())
    SYNC_PROP_MODIFY(side_axis, mSideAxis, SyncBones())
    SYNC_PROP_MODIFY(vertical, mVertical, SyncBones())
    SYNC_PROP_MODIFY(dest, mDest, SyncBones())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKRod)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mLeftEnd;
    bs << mRightEnd;
    bs << mDestPos;
    bs << mSideAxis;
    bs << mVertical;
    bs << mDest;
    bs << mXfm;
END_SAVES

BEGIN_COPYS(CharIKRod)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharIKRod)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLeftEnd)
        COPY_MEMBER(mRightEnd)
        COPY_MEMBER(mDestPos)
        COPY_MEMBER(mSideAxis)
        COPY_MEMBER(mVertical)
        COPY_MEMBER(mDest)
        COPY_MEMBER(mXfm)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharIKRod)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mLeftEnd;
    bs >> mRightEnd;
    bs >> mDestPos;
    bs >> mSideAxis;
    bs >> mVertical;
    bs >> mDest;
    bs >> mXfm;
END_LOADS

void CharIKRod::Poll() {
    Transform tf38;
    if (ComputeRod(tf38)) {
        Transform tf68;
        Multiply(mXfm, tf38, tf68);
        mDest->SetWorldXfm(tf68);
    }
}

void CharIKRod::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mDest);
    changedBy.push_back(mLeftEnd);
    changedBy.push_back(mRightEnd);
    changedBy.push_back(mSideAxis);
}

void CharIKRod::SyncBones() {
    Transform tf38;
    if (ComputeRod(tf38)) {
        Invert(tf38, tf38);
        Multiply(mDest->WorldXfm(), tf38, mXfm);
    }
}

bool CharIKRod::ComputeRod(Transform &tf) {
    if (mDest == 0 || mLeftEnd == 0 || mRightEnd == 0)
        return false;
    Interp(mLeftEnd->WorldXfm().v, mRightEnd->WorldXfm().v, mDestPos, tf.v);
    if (mVertical)
        tf.m.x.Set(0.0f, 0.0f, -1.0f);
    else {
        Interp(mLeftEnd->WorldXfm().m.x, mRightEnd->WorldXfm().m.x, mDestPos, tf.m.x);
        Normalize(tf.m.x, tf.m.x);
    }
    if (mSideAxis)
        tf.m.z = mSideAxis->WorldXfm().m.z;
    else
        Subtract(mLeftEnd->WorldXfm().v, mRightEnd->WorldXfm().v, tf.m.z);
    Cross(tf.m.z, tf.m.x, tf.m.y);
    Normalize(tf.m.y, tf.m.y);
    Cross(tf.m.x, tf.m.y, tf.m.z);
    return true;
}
