#include "char/CharIKSliderMidi.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharIKSliderMidi::CharIKSliderMidi()
    : mTarget(this), mFirstSpot(this), mSecondSpot(this), unk9c(1), unka4(0), unka8(0),
      unkac(0), mResetAll(1), mTolerance(0) {
    RndPollable::Enter();
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
