#include "char/CharCuff.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

CharCuff::CharCuff() : mOpenEnd(0), mIgnore(this), mBone(this), mEccentricity(1) {
    mShape[0].offset = -2.9;
    mShape[0].radius = 1.9;
    mShape[1].offset = 0;
    mShape[1].radius = 2.6;
    mShape[2].offset = 2.0;
    mShape[2].radius = 3.5;
    mOuterRadius = 3.1;
}

CharCuff::~CharCuff() {}

BEGIN_HANDLERS(CharCuff)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharCuff)
    SYNC_PROP(offset0, mShape[0].offset)
    SYNC_PROP(radius0, mShape[0].radius)
    SYNC_PROP(offset1, mShape[1].offset)
    SYNC_PROP(radius1, mShape[1].radius)
    SYNC_PROP(offset2, mShape[2].offset)
    SYNC_PROP(radius2, mShape[2].radius)
    SYNC_PROP(outer_radius, mOuterRadius)
    SYNC_PROP(open_end, mOpenEnd)
    SYNC_PROP(bone, mBone)
    SYNC_PROP(eccentricity, mEccentricity)
    SYNC_PROP(category, mCategory)
    SYNC_PROP(ignore, mIgnore)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharCuff)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    for (int i = 0; i < 3; i++) {
        bs << mShape[i].radius;
        bs << mShape[i].offset;
    }
    bs << mOuterRadius;
    bs << mOpenEnd;
    bs << mBone;
    bs << mEccentricity;
    bs << mCategory;
    bs << mIgnore;
END_SAVES

BEGIN_COPYS(CharCuff)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(CharCuff)
    BEGIN_COPYING_MEMBERS
        memcpy(mShape, c->mShape, sizeof(mShape));
        COPY_MEMBER(mOuterRadius)
        COPY_MEMBER(mOpenEnd)
        COPY_MEMBER(mBone)
        COPY_MEMBER(mEccentricity)
        COPY_MEMBER(mCategory)
        COPY_MEMBER(mIgnore)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharCuff)
    LOAD_REVS(bs)
    ASSERT_REVS(8, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    for (int i = 0; i < 3; i++) {
        bs >> mShape[i].radius >> mShape[i].offset;
    }
    if (d.rev > 1)
        bs >> mOuterRadius;
    else
        mOuterRadius = mShape[1].radius + 0.5f;
    if (d.rev > 2)
        bs >> mOpenEnd;
    else
        mOpenEnd = false;
    if (d.rev > 3)
        bs >> mBone;
    else
        mBone = TransParent();
    if (d.rev > 4)
        bs >> mEccentricity;
    else
        mEccentricity = 1.0f;
    if (d.rev > 5)
        bs >> mCategory;
    else
        mCategory = Symbol("");
    if (d.rev > 7)
        bs >> mIgnore;
    if (d.rev < 7)
        MILO_NOTIFY("%s old CharCuff, must convert, see James", PathName(this));
END_LOADS
