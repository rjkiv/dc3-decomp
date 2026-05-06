#include "char/CharEyes.h"
#include "char/CharInterest.h"
#include "char/CharLookAt.h"
#include "char/CharWeightable.h"
#include "math/Easing.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Std.h"

static const float sFloats[] = { 30, 3, 1 };

float chareyesdummyfunclmao() { return sFloats[0]; }

bool CharEyes::CharInterestState::IsInRefractoryPeriod() {
    if (mInterest && unk14 >= 0) {
        float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime) - unk14;
        if (secs < mInterest->RefractoryPeriod()) {
            return true;
        }
    }
    return false;
}

float CharEyes::CharInterestState::RefractoryTimeRemaining() {
    if (mInterest && unk14 >= 0) {
        float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime) - unk14;
        if (secs < mInterest->RefractoryPeriod()) {
            return mInterest->RefractoryPeriod() - secs;
        }
    }
    return 0;
}

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

bool CharEyes::Replace(ObjRef *from, Hmx::Object *to) {
    if (mEyes.size() != 0) {
        EyeDesc *cur = &mEyes[0];
        int diff = from - &cur->mEye;
        if (diff < mEyes.size()) {
            cur = &mEyes[diff / sizeof(EyeDesc)];
        }
        if (!cur->mEye.SetObj(to)) {
            mEyes.erase(cur);
        }
        return true;
    } else {
        return CharWeightable::Replace(from, to);
    }
}

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

BinStream &operator<<(BinStream &bs, const CharEyes::CharInterestState &state) {
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

BinStreamRev &operator>>(BinStreamRev &d, CharEyes::EyeDesc &desc) {
    d >> desc.mEye;
    d >> desc.mUpperLid;
    if (d.rev > 6) {
        d >> desc.mLowerLid;
    }
    if (d.rev > 15) {
        d >> desc.mUpperLidBlink;
        d >> desc.mLowerLidBlink;
    }
    return d;
}

BinStream &operator>>(BinStream &bs, CharEyes::CharInterestState &s) {
    bs >> s.mInterest;
    return bs;
}

INIT_REVS(18, 0)

BEGIN_LOADS(CharEyes)
    LOAD_REVS(bs)
    ASSERT_REVS(18, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 5) {
        LOAD_SUPERCLASS(CharWeightable)
    }
    if (d.rev > 4) {
        d >> mEyes;
    } else {
        ObjPtrList<CharLookAt> lookats(this);
        d >> lookats;
        mEyes.resize(lookats.size());
        int idx = 0;
        FOREACH (it, lookats) {
            mEyes[idx].mEye = *it;
            mEyes[idx].mUpperLid = nullptr;
            mEyes[idx].mLowerLid = nullptr;
            mEyes[idx].mUpperLidBlink = nullptr;
            mEyes[idx].mLowerLidBlink = nullptr;
            ++idx;
        }
    }
    if (d.rev > 2 && d.rev < 5) {
        ObjPtr<RndTransformable> trans(this);
        d >> trans;
    }
    mInterests.clear();
    if (d.rev > 3 && d.rev <= 8) {
        ObjPtr<RndTransformable> trans(this);
        int count;
        d >> count;
        for (int i = 0; i < count; i++) {
            d >> trans;
            int x;
            d >> x;
        }
    } else if (d.rev > 8) {
        d >> mInterests;
    }
    if (d.rev > 4) {
        d >> mFaceServo;
    } else {
        mFaceServo = nullptr;
    }
    if (d.rev > 7) {
        d >> mCamWeight;
    }
    if (d.rev > 9) {
        d >> mDefaultFilterFlags;
    }
    if (d.rev > 10) {
        d >> mViewDirection;
    }
    if (d.rev > 11) {
        d >> mHeadLookAt;
    }
    if (d.rev > 12) {
        d >> mMaxExtrapolation;
    }
    if (d.rev > 13) {
        d >> mMinTargetDist;
    }
    if (d.rev > 14) {
        d >> mUpperLidTrackUp >> mUpperLidTrackDown >> mLowerLidTrackUp;
        if (d.rev < 17) {
            int x, y;
            d >> x >> mLowerLidTrackDown >> y;
        } else {
            d >> mLowerLidTrackDown;
        }
    }
    if (d.rev > 17) {
        d >> mLowerLidTrackRotate;
    }
END_LOADS

void CharEyes::Enter() {
    unkd8.Zero();
    unkfc = false;
    unk170 = false;
    unk178 = -1;
    unkec = 0;
    unk18c = false;
    unkf4 = 0;
    unk194 = 0;
    unke8 = 1;
    unkfd = false;
    unkf8 = -1;
    unk174 = -1;
    unk190 = -1;
    unk198 = -1;
    unk19c = -1;
    mInterestFilterFlags = mDefaultFilterFlags;
    unk140 = 0;
    unk1b0 = false;
    unk12c = false;
    RndTransformable *head = GetHead();
    if (head) {
        unkd8 = head->WorldXfm().m.y;
        Normalize(unkd8, unkd8);
    }
    FOREACH (it, mEyes) {
        it->mEye->Enter();
    }
    FOREACH (it, mInterests) {
        it->unk14 = 0;
    }
    RndPollable::Enter();
}

void CharEyes::Exit() {
    unk114 = nullptr;
    unk128 = -1;
    mInterests.clear();
    FOREACH (it, mEyes) {
        it->mEye->Exit();
    }
    RndPollable::Exit();
}

void CharEyes::ListPollChildren(std::list<RndPollable *> &children) const {
    FOREACH (it, mEyes) {
        children.push_back(it->mEye);
    }
}

RndTransformable *CharEyes::GetTarget() {
    if (mEyes.empty() || !mEyes[0].mEye)
        return nullptr;
    else {
        return mEyes[0].mEye->Target();
    }
}

void CharEyes::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    FOREACH (it, mInterests) {
        ObjectDir *dir = it->mInterest->Dir();
        if (dir == Dir()) {
            changedBy.push_back(it->mInterest);
        }
    }
    if (!mEyes.empty()) {
        changedBy.push_back(GetHead());
        change.push_back(GetTarget());
    }
    if (mHeadLookAt) {
        changedBy.push_back(mHeadLookAt);
    }
    if (mFaceServo) {
        changedBy.push_back(mFaceServo);
    }
}

void CharEyes::ForceBlink() {
    if (unk1b1 && !unk18c) {
        unk18c = true;
        unk190 = TheTaskMgr.Seconds(TaskMgr::kRealTime);
        unk194++;
    }
}

void CharEyes::SetEnableBlinks(bool b1, bool b2) {
    unk1b1 = b1;
    if (b2 && !b1 && unk18c && mFaceServo) {
        mFaceServo->SetProceduralBlinkWeight(0.0f);
        unk18c = false;
        unk78 = unk1a0;
    }
}

bool CharEyes::SetFocusInterest(CharInterest *interest, int i) {
    if (unk114 && unk128 > i) {
        return false;
    } else {
        unk114 = interest;
        unk128 = i;
        if (unk114 != interest) {
            unk12c = true;
        }
        if (!unk114) {
            unk128 = -1;
        }

        return true;
    }
}

void CharEyes::AddInterestObject(CharInterest *interest) {
    if (interest) {
        CharInterestState state(this);
        state.mInterest = interest;
        mInterests.push_back(state);
    }
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

RndTransformable *CharEyes::GetHead() {
    if (mViewDirection) {
        return mViewDirection;
    } else if (!mEyes.empty() && mEyes[0].mEye) {
        RndTransformable *src = mEyes[0].mEye->GetSource();
        if (src) {
            return src->TransParent();
        }
    }
    return nullptr;
}

CharInterest *CharEyes::GetCurrentInterest() {
    if (unk114) {
        return unk114;
    } else if (unk100) {
        return unk100;
    } else {
        return nullptr;
    }
}

void CharEyes::ProceduralBlinkUpdate() {
    static DataNode &disableCheat = DataVariable("cheat.disable_procedural_blinks");

    if (!sDisableProceduralBlink && disableCheat.Int() == 0 && (unk1b1 || unk18c)) {
        unk198 = unk198 - TheTaskMgr.DeltaSeconds();
        if (unk198 < 0.0f) {
            unk194 = 0;
            unk198 = 15.0f;
        }
        if (mFaceServo && unk18c) {
            float elapsed = TheTaskMgr.Seconds(TaskMgr::kRealTime) - unk190;
            if (elapsed < 0.115f) {
                // Closing phase
                float ease = EaseInExp(Clamp(0.0f, 1.0f, elapsed * 8.695652f));
                mFaceServo->SetProceduralBlinkWeight(ease);
            } else if (elapsed < 0.3f) {
                // Opening phase
                float t = Clamp(0.0f, 1.0f, 1.0f - (elapsed - 0.115f) * 5.4054055f);
                float ease = EaseSigmoid(t, 0, 0);
                mFaceServo->SetProceduralBlinkWeight(ease);
                unk78 = unk1a0;
            } else {
                // Blink complete
                mFaceServo->SetProceduralBlinkWeight(0.0f);
                unk18c = false;
                unk78 = unk1a0;
            }
        }
    }
}

void CharEyes::EnforceMinimumTargetDistance(
    const Vector3 &v1, const Vector3 &v2, Vector3 &vres
) {
    Vector3 sub;
    Subtract(v2, v1, sub);
    float len = Length(sub);
    unkfd = false;
    float f1;
    if (unk100 && unk100->OverridesMinTargetDist()) {
        f1 = unk100->MinTargetDistOverride();
    } else {
        f1 = mMinTargetDist;
    }
    if (len < f1) {
        Vector3 scaled;
        NormalizeScale(sub, f1, scaled);
        Add(v1, scaled, vres);
        unkfd = true;
    }
}

void CharEyes::DartUpdate() {
    static DataNode &n = DataVariable("cheat.disable_eye_darts");
    if (!sDisableEyeDart && n.Int() == 0) {
        unk174 -= TheTaskMgr.DeltaSeconds();
        if (unk170) {
            if (unk174 < 0) {
                if (--unk178 < 0) {
                    unk170 = false;
                    unk174 = RandomFloat(
                        mData.mMinSecsBetweenSequences, mData.mMaxSecsBetweenSequences
                    );
                } else {
                    unk174 = RandomFloat(
                        mData.mMinSecsBetweenDarts, mData.mMaxSecsBetweenDarts
                    );
                    unk17c = GenerateDartOffset();
                }
            }
        } else {
            if (unk174 < 0 && EyesOnTarget(mData.mOnTargetAngleThresh) && !unk18c) {
                unk170 = true;
                unk178 =
                    RandomInt(mData.mMinDartsPerSequence, mData.mMaxDartsPerSequence);
                unk174 =
                    RandomFloat(mData.mMinSecsBetweenDarts, mData.mMaxSecsBetweenDarts);
                unk17c = GenerateDartOffset();
            }
        }
    }
}

Vector3 CharEyes::GenerateDartOffset() {
    float min = mData.mMinRadius;
    float max = mData.mMaxRadius;
    if (mData.mScaleWithDistance && mData.mReferenceDistance > 0.1f) {
        Vector3 sub;
        Subtract(unk78, GetHead()->WorldXfm().v, sub);
        float scalar = Length(sub) / mData.mReferenceDistance;
        min *= scalar;
        max *= scalar;
    }
    Vector3 v;
    v[0] = RandomFloat(min, max) * (RandomFloat(0, 1) > 0.5f ? 1.0f : -1.0f);
    v[1] = RandomFloat(min, max) * (RandomFloat(0, 1) > 0.5f ? 1.0f : -1.0f);
    v[2] = RandomFloat(min, max) * (RandomFloat(0, 1) > 0.5f ? 1.0f : -1.0f);
    return v;
}

bool CharEyes::EyesOnTarget(float f) {
    FOREACH (it, mEyes) {
        RndTransformable *src = it->mEye->GetSource();
        if (src) {
            Vector3 v80;
            Subtract(unk78, src->WorldXfm().v, v80);
            Vector3 v8c(src->WorldXfm().m.y);
            Vector3 v98(v80);
            v98.z = 0;
            v8c.z = 0;
            float dot = Dot(v8c, v98);
            if (acosf(Clamp<float>(-1, 1, dot / (Length(v8c) * Length(v98)))) * 57.29578f
                > f) {
                return false;
            }
        }
    }
    return true;
}

void CharEyes::UpdateOverlay() {
    if (mEyeStatusOverlay && mEyeStatusOverlay->Showing()) {
        *mEyeStatusOverlay << Dir()->Name() << ": ";
        if (unk100) {
            if (unk114 && streq(unk100->Name(), unk114->Name())) {
                *mEyeStatusOverlay << "Look(FOC) ";
            } else {
                *mEyeStatusOverlay << "Look(" << unk100->Name() << ") ";
            }
        } else {
            *mEyeStatusOverlay << "Look(GEN) ";
        }
        if (unk114) {
            const Transform &headXfm = GetHead()->WorldXfm();
            Vector3 v = headXfm.m.y;
            Normalize(v, v);
            const char *str = unk114->IsWithinViewCone(headXfm.v, v) ? "t" : "f";
            *mEyeStatusOverlay << "Foc(" << unk114->Name() << " p(" << unk128 << ") v("
                               << str << ")) ";
        } else {
            *mEyeStatusOverlay << "Foc(NA) ";
        }
        *mEyeStatusOverlay << "t(" << unkec << ") ";
        Vector3 headV = GetHead()->WorldXfm().v;
        Vector3 v4c;
        Vector3 v58(unk78);
        RndTransformable *target = GetTarget();
        if (target) {
            v58 = target->WorldXfm().v;
        }
        Subtract(v58, headV, v4c);
        float len = Length(v4c);
        *mEyeStatusOverlay << "Dist(" << len << ") ";
        if (unk18c) {
            *mEyeStatusOverlay << "P Blink! ";
        }
        if (unk170) {
            *mEyeStatusOverlay << "Dart! ";
        }
        if (unkfd) {
            *mEyeStatusOverlay << "Close! ";
        }
        *mEyeStatusOverlay << "\n";
    }
}

DataNode CharEyes::OnAddInterest(DataArray *arr) {
    mInterests.push_back(CharInterestState(arr->Obj<CharInterest>(1)));
    return 0;
}

DataNode CharEyes::OnToggleForceFocus(DataArray *) {
    if (unk114)
        SetFocusInterest(nullptr, 0);
    else
        SetFocusInterest(unk100, 0);
    return 0;
}

DataNode CharEyes::OnToggleInterestOverlay(DataArray *) {
    ToggleInterestsDebugOverlay();
    return 0;
}
