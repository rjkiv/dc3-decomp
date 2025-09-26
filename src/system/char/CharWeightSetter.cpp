#include "char/CharWeightSetter.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharWeightSetter::CharWeightSetter()
    : mBase(this), mDriver(this), mMinWeights(this), mMaxWeights(this), mFlags(0),
      mOffset(0), mScale(1), mBaseWeight(0), mBeatsPerWeight(0) {}

BEGIN_HANDLERS(CharWeightSetter)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharWeightSetter)
    SYNC_PROP(driver, mDriver)
    SYNC_PROP(flags, mFlags)
    SYNC_PROP(base, mBase)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(scale, mScale)
    SYNC_PROP(base_weight, mBaseWeight)
    SYNC_PROP(beats_per_weight, mBeatsPerWeight)
    SYNC_PROP(min_weights, mMinWeights)
    SYNC_PROP(max_weights, mMaxWeights)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharWeightSetter)
    SAVE_REVS(9, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mDriver;
    bs << mFlags;
    bs << mOffset;
    bs << mScale;
    bs << mBaseWeight;
    bs << mBeatsPerWeight;
    bs << mBase;
    bs << mMinWeights;
    bs << mMaxWeights;
END_SAVES

BEGIN_COPYS(CharWeightSetter)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharWeightSetter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDriver)
        COPY_MEMBER(mFlags)
        COPY_MEMBER(mBase)
        COPY_MEMBER(mOffset)
        COPY_MEMBER(mScale)
        COPY_MEMBER(mBaseWeight)
        COPY_MEMBER(mBeatsPerWeight)
        COPY_MEMBER(mMinWeights)
        COPY_MEMBER(mMaxWeights)
    END_COPYING_MEMBERS
END_COPYS

void CharWeightSetter::SetWeight(float weight) {
    mBaseWeight = weight;
    mWeight = weight;
}
