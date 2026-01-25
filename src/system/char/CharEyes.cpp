#include "char/CharEyes.h"
#include "char/CharInterest.h"
#include "char/CharWeightable.h"
#include "math/Easing.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "utl/BinStream.h"

CharEyes::CharEyes()
    : mEyes(this), mInterests(this), mFaceServo(this), mCamWeight(this), unk78(0, 0, 0),
      mDefaultFilterFlags(0), mViewDirection(this), mHeadLookAt(this),
      mMaxExtrapolation(19.5), mMinTargetDist(35), mUpperLidTrackUp(1),
      mUpperLidTrackDown(1), mLowerLidTrackUp(0.75), mLowerLidTrackDown(0.75),
      mLowerLidTrackRotate(false), mInterestFilterFlags(0), unkd8(0, 0, 0), unkec(0),
      unkf8(0), unkfc(0), unkfd(0), unk100(this), unk114(this), unk128(-1), unk12c(0),
      unk130(0, 1, 0), unk140(0), unk170(0), unk174(-1), unk178(-1), unk18c(0),
      unk190(-1), unk194(0), unk198(-1), unk19c(-1), unk1b0(0), unk1b1(1) {
    unkf0 = std::cos(0.5235987715423107);
    mEyeStatusOverlay = RndOverlay::Find("eye_status", false);
}

CharEyes::~CharEyes() {}

BEGIN_HANDLERS(CharEyes)
    HANDLE(add_interest, OnAddInterest)
    HANDLE_ACTION(force_blink, ForceBlink())
    HANDLE(toggle_force_focus, OnToggleForceFocus)
    HANDLE(toggle_interest_overlay, OnToggleInterestOverlay)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharEyes::EyeDesc)
    SYNC_PROP(eye, o.mEye)
    SYNC_PROP(upper_lid, o.mUpperLid)
    SYNC_PROP(lower_lid, o.mLowerLid)
    SYNC_PROP(upper_lid_blink, o.mUpperLidBlink)
    SYNC_PROP(lower_lid_blink, o.mLowerLidBlink)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(CharEyes::CharInterestState)
    SYNC_PROP(interest, o.mInterest)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharEyes)
    SYNC_PROP(eyes, mEyes)
    SYNC_PROP(view_direction, mViewDirection)
    SYNC_PROP(interests, mInterests)
    SYNC_PROP(face_servo, mFaceServo)
    SYNC_PROP(camera_weight, mCamWeight)
    SYNC_PROP_BITFIELD(default_interest_categories, mDefaultFilterFlags, 0x685)
    SYNC_PROP(head_lookat, mHeadLookAt)
    SYNC_PROP(max_extrapolation, mMaxExtrapolation)
    SYNC_PROP(disable_eye_dart, sDisableEyeDart)
    SYNC_PROP(disable_eye_jitter, sDisableEyeJitter)
    SYNC_PROP(disable_interest_objects, sDisableInterestObjects)
    SYNC_PROP(disable_procedural_blink, sDisableProceduralBlink)
    SYNC_PROP(disable_eye_clamping, sDisableEyeClamping)
    SYNC_PROP_BITFIELD(interest_filter_testing, mInterestFilterFlags, 0x68E)
    SYNC_PROP(min_target_dist, mMinTargetDist)
    SYNC_PROP(ulid_track_up, mUpperLidTrackUp)
    SYNC_PROP(ulid_track_down, mUpperLidTrackDown)
    SYNC_PROP(llid_track_up, mLowerLidTrackUp)
    SYNC_PROP(llid_track_down, mLowerLidTrackDown)
    SYNC_PROP(llid_track_rotate, mLowerLidTrackRotate)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharEyes::EyeDesc &desc) {
    bs << desc.mEye;
    bs << desc.mUpperLid;
    bs << desc.mLowerLid;
    bs << desc.mUpperLidBlink;
    bs << desc.mLowerLidBlink;
    return bs;
}

inline BinStream &operator<<(BinStream &bs, const CharEyes::CharInterestState &state) {
    bs << state.mInterest;
    return bs;
}

BEGIN_SAVES(CharEyes)
    SAVE_REVS(18, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mEyes;
    bs << mInterests;
    bs << mFaceServo;
    bs << mCamWeight;
    bs << mDefaultFilterFlags;
    bs << mViewDirection;
    bs << mHeadLookAt;
    bs << mMaxExtrapolation;
    bs << mMinTargetDist;
    bs << mUpperLidTrackUp;
    bs << mUpperLidTrackDown;
    bs << mLowerLidTrackUp;
    bs << mLowerLidTrackDown;
    bs << mLowerLidTrackRotate;
END_SAVES

BEGIN_COPYS(CharEyes)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharEyes)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mEyes)
        COPY_MEMBER(mInterests)
        COPY_MEMBER(mFaceServo)
        COPY_MEMBER(unkd8)
        COPY_MEMBER(unkec)
        COPY_MEMBER(mCamWeight)
        COPY_MEMBER(mDefaultFilterFlags)
        COPY_MEMBER(mViewDirection)
        COPY_MEMBER(mHeadLookAt)
        COPY_MEMBER(mMaxExtrapolation)
        COPY_MEMBER(mMinTargetDist)
        COPY_MEMBER(mUpperLidTrackUp)
        COPY_MEMBER(mUpperLidTrackDown)
        COPY_MEMBER(mLowerLidTrackUp)
        COPY_MEMBER(mLowerLidTrackDown)
        COPY_MEMBER(mLowerLidTrackRotate)
    END_COPYING_MEMBERS
END_COPYS

void CharEyes::ForceBlink() {
    if (unk1b1 && !unk18c) {
        unk18c = true;
        unk190 = TheTaskMgr.Seconds(TaskMgr::kRealTime);
        unk194++;
    }
}

void CharEyes::SetEnableBlinks(bool b1, bool b2) {
    unk1b1 = b1;
    if (!b2 || b1 || !unk18c || !mFaceServo)
        return;

    mFaceServo->SetProceduralBlinkWeight(0.0f);
    unk18c = false;
    unk78 = unk1a0;
}

bool CharEyes::SetFocusInterest(CharInterest *interest, int i) {
    if (unk114 && unk128 > i)
        return false;

    unk114 = interest;
    unk128 = i;
    if (interest != unk114)
        unk12c = true;
    if (!unk114)
        unk128 = -1;

    return true;
}

void CharEyes::ToggleInterestsDebugOverlay() {
    if (mEyeStatusOverlay)
        mEyeStatusOverlay->SetShowing(!mEyeStatusOverlay->Showing());
}

bool CharEyes::IsHeadIKWeightIncreasing() {
    if (mHeadLookAt) {
        float weight = mHeadLookAt->Weight();
        return (weight > 0 && weight - unk140 > 0);
    }
    return false;
}

void CharEyes::ClearAllInterestObjects() { mInterests.clear(); }

void CharEyes::ProceduralBlinkUpdate() {
    static DataNode &disableCheat = DataVariable("cheat.disable_procedural_blinks");

    if (sDisableProceduralBlink)
        return;
    if (disableCheat.Int(0))
        return;
    if (!unk1b1 && !unk18c)
        return;

    unk198 = unk198 - TheTaskMgr.DeltaSeconds();
    if (unk198 < 0.0f) {
        unk194 = 0;
        unk198 = 15.0f;
    }

    if (!mFaceServo)
        return;
    if (!unk18c)
        return;

    float elapsed = TheTaskMgr.Seconds(TaskMgr::kRealTime) - unk190;
    if (elapsed < 0.115f) {
        // Closing phase
        float t = Clamp(0.0f, 1.0f, elapsed * 8.695652f);
        mFaceServo->SetProceduralBlinkWeight(EaseInExp(t));
    } else if (elapsed < 0.3f) {
        // Opening phase
        float t = Clamp(0.0f, 1.0f, 1.0f - (elapsed - 0.115f) * 5.405405f);
        mFaceServo->SetProceduralBlinkWeight(EaseSigmoid(t, 0.0f, 0.0f));
        unk78 = unk1a0;
    } else {
        // Blink complete
        mFaceServo->SetProceduralBlinkWeight(0.0f);
        unk18c = false;
        unk78 = unk1a0;
    }
}
