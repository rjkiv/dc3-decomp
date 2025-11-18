#include "char/CharIKSliderMidi.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "obj/Task.h"

CharIKSliderMidi::CharIKSliderMidi()
    : mTarget(this), mFirstSpot(this), mSecondSpot(this) {
    mTargetPercentage = 1.0f;
    mTolerance = 0.0f;
    mPercentageChanged = false;
    mResetAll = true;
    Enter();
}

CharIKSliderMidi::~CharIKSliderMidi() {}

BEGIN_HANDLERS(CharIKSliderMidi)
    HANDLE_ACTION(set_fraction, SetFraction(_msg->Float(2), _msg->Float(3)))
    HANDLE_ACTION(reset, SetupTransforms())
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKSliderMidi)
    SYNC_PROP_MODIFY(target, mTarget, SetupTransforms())
    SYNC_PROP_MODIFY(first_spot, mFirstSpot, SetupTransforms())
    SYNC_PROP_MODIFY(second_spot, mSecondSpot, SetupTransforms())
    SYNC_PROP(tolerance, mTolerance)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKSliderMidi)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mTarget;
    bs << mFirstSpot;
    bs << mSecondSpot;
    bs << mTolerance;
END_SAVES

BEGIN_COPYS(CharIKSliderMidi)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKSliderMidi)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mFirstSpot)
        COPY_MEMBER(mSecondSpot)
        COPY_MEMBER(mTolerance)
    END_COPYING_MEMBERS
END_COPYS

void CharIKSliderMidi::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mTarget);
    changedBy.push_back(mTarget);
    changedBy.push_back(mFirstSpot);
    changedBy.push_back(mSecondSpot);
}

void CharIKSliderMidi::SetupTransforms() { mResetAll = true; }

void CharIKSliderMidi::Enter() {
    mPercentageChanged = false;
    mFrac = 0.0f;
    mFracPerBeat = 0.0f;
    RndPollable::Enter();
}

void CharIKSliderMidi::SetFraction(float f1, float f2) {
    if (f1 != mTargetPercentage) {
        if (std::fabs(f1 - mTargetPercentage) < mTolerance)
            return;
        else {
            mOldPercentage = mTargetPercentage;
            mTargetPercentage = f1;
            if (f2 <= 0)
                mFracPerBeat = kHugeFloat;
            else {
                MaxEq(f2, 0.1f);
                mFracPerBeat = 1.0f / f2;
            }
            mFrac = 0;
            mPercentageChanged = true;
        }
    }
}
