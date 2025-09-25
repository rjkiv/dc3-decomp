#include "char/CharIKHead.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharIKHead::CharIKHead()
    : mPoints(this), mHead(this), mSpine(this), mMouth(this), mTarget(this),
      mHeadFilter(0, 0, 0), mTargetRadius(0.75), mHeadMat(0.5), mOffset(this),
      mOffsetScale(1, 1, 1), mUpdatePoints(1) {}

CharIKHead::~CharIKHead() {}

BEGIN_HANDLERS(CharIKHead)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKHead)
    SYNC_PROP_MODIFY(head, mHead, UpdatePoints(true))
    SYNC_PROP_MODIFY(spine, mSpine, UpdatePoints(true))
    SYNC_PROP(mouth, mMouth)
    SYNC_PROP(target, mTarget)
    SYNC_PROP(target_radius, mTargetRadius)
    SYNC_PROP(head_mat, mHeadMat)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(offset_scale, mOffsetScale)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKHead)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mHead;
    bs << mSpine;
    bs << mMouth;
    bs << mTarget;
    bs << mTargetRadius;
    bs << mHeadMat;
    bs << mOffset;
    bs << mOffsetScale;
END_SAVES

BEGIN_COPYS(CharIKHead)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKHead)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHead)
        COPY_MEMBER(mSpine)
        COPY_MEMBER(mMouth)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mTargetRadius)
        COPY_MEMBER(mHeadMat)
        COPY_MEMBER(mOffset)
        COPY_MEMBER(mOffsetScale)
        mUpdatePoints = true;
    END_COPYING_MEMBERS
END_COPYS
