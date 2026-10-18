#include "char/CharSignalApplier.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

CharSignalApplier::CharSignalApplier()
    : mSignalValue(0), mSignalMin(-1.0f), mSignalMax(1.0f), mUseSmoothing(false),
      mPollSignalStep(0.1f), mSmoothedSignal(0), mBoneOps(this) {}

BEGIN_HANDLERS(CharSignalApplier)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharSignalApplier::BoneOp)
    SYNC_PROP(bone, o.mBone)
    SYNC_PROP(op, o.mOp)
    SYNC_PROP(apply_percent, o.mApplyPct)
    SYNC_PROP(min_angle, o.mMinAngle)
    SYNC_PROP(max_angle, o.mMaxAngle)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharSignalApplier)
    SYNC_PROP(bone_ops, mBoneOps)
    SYNC_PROP(signal, mSignalValue)
    SYNC_PROP(do_smoothing, mUseSmoothing)
    SYNC_PROP(smooth_increment, mPollSignalStep)
    SYNC_PROP(signal_min, mSignalMin)
    SYNC_PROP(signal_max, mSignalMax)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharSignalApplier::BoneOp &op) {
    bs << op.mBone;
    bs << op.mOp;
    bs << op.mApplyPct;
    bs << op.mMinAngle;
    bs << op.mMaxAngle;
    return bs;
}

BEGIN_SAVES(CharSignalApplier)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mBoneOps;
    bs << mSignalValue;
    bs << mSignalMin;
    bs << mSignalMax;
    bs << mUseSmoothing;
    bs << mPollSignalStep;
END_SAVES

BEGIN_COPYS(CharSignalApplier)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY_AS(CharSignalApplier, c)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBoneOps)
        COPY_MEMBER(mSignalValue)
        COPY_MEMBER(mSignalMin)
        COPY_MEMBER(mSignalMax)
        COPY_MEMBER(mUseSmoothing)
        COPY_MEMBER(mPollSignalStep)
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, CharSignalApplier::BoneOp &op) {
    d >> op.mBone;
    d >> op.mOp;
    d >> op.mApplyPct;
    d >> op.mMinAngle;
    d >> op.mMaxAngle;
    return d;
}

INIT_REVS(0, 0)

BEGIN_LOADS(CharSignalApplier)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mBoneOps;
    d >> mSignalValue;
    d >> mSignalMin;
    d >> mSignalMax;
    d >> mUseSmoothing;
    d >> mPollSignalStep;
END_LOADS

void CharSignalApplier::Poll() {}

void CharSignalApplier::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &
) {
    FOREACH (it, mBoneOps) {
        BoneOp op(*it);
        changedBy.push_back(op.mBone);
    }
}
