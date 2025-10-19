#include "char/CharBone.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

BinStream &operator<<(BinStream &bs, const CharBone::WeightContext &ctx) {
    bs << ctx.mContext;
    bs << ctx.mWeight;
    return bs;
}

BEGIN_CUSTOM_PROPSYNC(CharBone::WeightContext)
    SYNC_PROP(context, o.mContext)
    SYNC_PROP(weight, o.mWeight)
END_CUSTOM_PROPSYNC

void CharBone::ClearContext(int i) {
    int mask = ~i;
    mPositionContext &= mask;
    mScaleContext &= mask;
    mRotationContext &= mask;
}

BEGIN_HANDLERS(CharBone)
    HANDLE_ACTION(clear_context, ClearContext(_msg->Int(2)))
    HANDLE(get_context_flags, OnGetContextFlags)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_SAVES(CharBone)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPositionContext;
    bs << mScaleContext;
    bs << mRotation;
    bs << mRotationContext;
    bs << mTarget;
    bs << mWeights;
    bs << mTrans;
    bs << mBakeOutAsTopLevel;
END_SAVES

BEGIN_PROPSYNCS(CharBone)
    SYNC_PROP(position_context, mPositionContext)
    SYNC_PROP(scale_context, mScaleContext)
    SYNC_PROP(rotation, (int &)mRotation)
    SYNC_PROP(rotation_context, mRotationContext)
    SYNC_PROP(target, mTarget)
    SYNC_PROP(weights, mWeights)
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(bake_out_as_top_level, mBakeOutAsTopLevel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

CharBone::CharBone()
    : mPositionContext(0), mScaleContext(0), mRotation(CharBones::TYPE_END),
      mRotationContext(0), mTarget(this), mWeights(), mTrans(this),
      mBakeOutAsTopLevel(0) {}

BEGIN_LOADS(CharBone)
    LOAD_REVS(bs)
    ASSERT_REVS(10, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 9) {
        RndTransformableRemover t;
        t.Load(bs);
    }
END_LOADS

BEGIN_COPYS(CharBone)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBone)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mRotationContext)
        COPY_MEMBER(mScaleContext)
        COPY_MEMBER(mPositionContext)
        COPY_MEMBER(mRotation)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mWeights)
        COPY_MEMBER(mTrans)
        COPY_MEMBER(mBakeOutAsTopLevel)
    END_COPYING_MEMBERS
END_COPYS
