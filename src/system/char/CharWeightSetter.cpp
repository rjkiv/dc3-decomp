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

BEGIN_LOADS(CharWeightSetter)
    LOAD_REVS(bs)
    ASSERT_REVS(9, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 1)
        LOAD_SUPERCLASS(CharWeightable)
    bs >> mDriver;
    bs >> mFlags;
    if (d.rev < 3) {
        mScale = 1.0f;
        mOffset = 0.0f;
    } else if (d.rev < 4) {
        bool b;
        d >> b;
        if (b) {
            mScale = -1.0f;
            mOffset = 1.0f;
        } else {
            mScale = 1.0f;
            mOffset = 0.0f;
        }
    } else
        bs >> mOffset >> mScale;
    if (d.rev < 2) {
        ObjPtrList<CharWeightable, ObjectDir> pList(this, kObjListNoNull);
        bs >> pList;
        for (ObjPtrList<CharWeightable, ObjectDir>::iterator it = pList.begin();
             it != pList.end();
             ++it) {
            (*it)->SetWeightOwner(this);
        }
    }
    if (d.rev > 4) {
        bs >> mBaseWeight;
        bs >> mBeatsPerWeight;
    } else {
        mBaseWeight = mWeight;
        mBeatsPerWeight = 0.0f;
    }
    if (d.rev > 5)
        bs >> mBase;
    if (d.rev > 8) {
        bs >> mMinWeights;
        bs >> mMaxWeights;
    } else {
        if (d.rev > 6) {
            ObjPtr<CharWeightSetter> ptrWS(this, 0);
            bs >> ptrWS;
            if (ptrWS)
                mMinWeights.push_back(ptrWS);
        }
        if (d.rev > 7) {
            ObjPtr<CharWeightSetter> ptrWS(this, 0);
            bs >> ptrWS;
            if (ptrWS)
                mMaxWeights.push_back(ptrWS);
        }
    }
END_LOADS

void CharWeightSetter::SetWeight(float weight) {
    mBaseWeight = weight;
    mWeight = weight;
}

void CharWeightSetter::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mDriver);
    changedBy.push_back(mBase);
    for (ObjPtrList<CharWeightSetter>::iterator it = mMinWeights.begin();
         it != mMinWeights.end();
         ++it) {
        changedBy.push_back(*it);
    }
    for (ObjPtrList<CharWeightSetter>::iterator it = mMaxWeights.begin();
         it != mMaxWeights.end();
         ++it) {
        changedBy.push_back(*it);
    }
    FOREACH (it, Refs()) {
        CharWeightable *weightowner = dynamic_cast<CharWeightable *>((*it).RefOwner());
        if (weightowner && weightowner->WeightOwner() == this)
            change.push_back(weightowner);
    }
}

void CharWeightSetter::Poll() {
    if (mDriver) {
        mBaseWeight = mScale * mDriver->EvaluateFlags(mFlags) + mOffset;
    } else if (mBase) {
        mBaseWeight = mScale * mBase->Weight() + mOffset;
    }

    if (mMinWeights.size() > 0) {
        float newminweight = mBaseWeight;
        for (ObjPtrList<CharWeightSetter>::iterator it = mMinWeights.begin();
             it != mMinWeights.end();
             ++it) {
            MinEq(newminweight, (*it)->Weight());
        }
        mBaseWeight = newminweight;
    }

    if (mMaxWeights.size() > 0) {
        float newmaxweight = mBaseWeight;
        for (ObjPtrList<CharWeightSetter>::iterator it = mMaxWeights.begin();
             it != mMaxWeights.end();
             ++it) {
            MaxEq(newmaxweight, (*it)->Weight());
        }
        mBaseWeight = newmaxweight;
    }

    if (mBaseWeight != mWeight) {
        if (mBeatsPerWeight <= 0.0f)
            mWeight = mBaseWeight;
        else {
            float secs = TheTaskMgr.DeltaBeat() / mBeatsPerWeight;
            if (secs > 0.0f) {
                float clamped = Clamp(-secs, secs, mBaseWeight - mWeight);
                mWeight += clamped;
            }
        }
    }
}
