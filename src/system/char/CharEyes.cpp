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
#include "rndobj/Cam.h"
#include "rndobj/Graph.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"
#include "utl/Std.h"
#include "world/Dir.h"

static const float sFloats[] = { 30, 3, 1 };

float chareyesdummyfunclmao() { return sFloats[0]; }

bool CharEyes::CharInterestState::IsInRefractoryPeriod() {
    if (mInterest && unk14 >= 0) {
        float secs = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) - unk14;
        if (secs < mInterest->RefractoryPeriod()) {
            return true;
        }
    }
    return false;
}

float CharEyes::CharInterestState::RefractoryTimeRemaining() {
    if (mInterest && unk14 >= 0) {
        float secs = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) - unk14;
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
      mLowerLidTrackRotate(false), mInterestFilterFlags(0), mLastFacing(0, 0, 0), mLastLook(0),
      mLastBlinkWeight(0), mBlinkDetect(0), mTargetTooClose(0), mCurInterest(this), mFocusInterest(this), mCurFocusPriorityClass(-1), mNewFocusInterest(0),
      mLastExtrapolatedDir(0, 1, 0), mLastHeadIKWeight(0), mDarting(0), mDartNextEventTime(-1), mDartsRemaining(-1), mProceduralBlink(0),
      mBlinkTimestamp(-1), mBlinkWindowCount(0), mBlinkWindowSecsRemaining(-1), mLastBlinkDetectTime(-1), mInterestFiltersChanged(0), mBlinksEnabled(1) {
    mMaxEyeCang = std::cos(0.5235987715423107);
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
        COPY_MEMBER(mLastFacing)
        COPY_MEMBER(mLastLook)
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

void CharEyes::Highlight() {
    if (GetHead()) {
        RndGraph *oneframe = RndGraph::GetOneFrame();
        RndTransformable *trans = nullptr;
        FOREACH (it, mEyes) {
            trans = it->mEye->GetSource();
            if (trans) {
                const Transform &tf1 = trans->WorldXfm();
                const Transform &tf2 = trans->WorldXfm();
                Vector3 v100;
                ScaleAdd(tf2.v, tf1.m.y, 3, v100);
                if (it->mEye->Clamped())
                    oneframe->AddLine(
                        trans->WorldXfm().v, v100, Hmx::Color(1.0f, 0.0f, 0.0f), true
                    );
                else
                    oneframe->AddLine(
                        trans->WorldXfm().v, v100, Hmx::Color(0.0f, 1.0f, 0.0f), true
                    );
            }
        }
        Vector3 v10c(GetHead()->WorldXfm().v);
        if (trans) {
            bool fcmp = mLastCang >= (mCurInterest ? mCurInterest->MaxViewAngleCos() : mMaxEyeCang);
            if (mDarting) {
                oneframe->AddSphere(unk78, mCurDartRuleset.mMaxRadius, Hmx::Color(0.9f, 0.9f, 0.9f));
                Vector3 v118;
                Add(unk78, mCurDartOffset, v118);
                EnforceMinimumTargetDistance(v10c, v118, v118);
                oneframe->AddSphere(v118, 0.5f, Hmx::Color(0.0f, 0.0f, 1.0f));
                oneframe->AddLine(
                    trans->WorldXfm().v,
                    v118,
                    fcmp ? Hmx::Color(0.2f, 0.2f, 1.0f) : Hmx::Color(1, 0, 0),
                    true
                );
            } else {
                oneframe->AddLine(
                    trans->WorldXfm().v,
                    unk78,
                    fcmp ? Hmx::Color(1, 1, 1) : Hmx::Color(1, 0, 0),
                    true
                );
            }
            if (mProceduralBlink) {
                oneframe->AddString3D(
                    "p blink!", trans->WorldXfm().v, Hmx::Color(1, 1, 1)
                );
            }
        }

        if (mFocusInterest) {
            if (mFocusInterest != mCurInterest) {
                const char *nametouse = mCurInterest ? mCurInterest->Name() : "GENERATED";
                oneframe->AddString3D(
                    MakeString("focus = '%s' (looking at %s)", mFocusInterest->Name(), nametouse),
                    v10c,
                    Hmx::Color(1, 0, 0)
                );
            } else {
                oneframe->AddString3D(
                    MakeString("focus = '%s'", mFocusInterest->Name()), v10c, Hmx::Color(0, 1, 0)
                );
            }
        } else {
            if (mCurInterest) {
                oneframe->AddString3D(
                    MakeString("interest = '%s'", mCurInterest->Name()),
                    v10c,
                    Hmx::Color(0, 1, 0)
                );
            }
        }

        if (mInterests.size() != 0) {
            const Transform &headXfm = GetHead()->WorldXfm();
            Vector3 headMY = headXfm.m.y;
            Normalize(headMY, headMY);
            Vector3 va0 = headXfm.v;
            FOREACH (it, mInterests) {
                bool b7 = it->mInterest->IsMatchingFilterFlags(mInterestFilterFlags)
                    || ((mInterestFilterFlags == mDefaultFilterFlags)
                        && !it->mInterest->CategoryFlags());
                if (mCurInterest == it->mInterest) {
                    oneframe->AddSphere(
                        it->mInterest->WorldXfm().v, 2, Hmx::Color(0, 1, 0)
                    );
                    Vector2 v2;
                    if (RndCam::Current()->WorldToScreen(it->mInterest->WorldXfm().v, v2)
                        > 0) {
                        v2.x *= TheRnd.Width();
                        v2.y *= TheRnd.Height();
                        v2.y += 15.0;
                        v2.x -= 30.0;
                        oneframe->AddString(
                            MakeString("%s", it->mInterest->Name()),
                            v2,
                            Hmx::Color(1, 1, 1)
                        );
                    }
                } else {
                    if (it->mInterest->IsWithinViewCone(va0, mLastExtrapolatedDir)
                        && it->mInterest->IsWithinViewCone(va0, headMY)) {
                        oneframe->AddSphere(
                            it->mInterest->WorldXfm().v,
                            2,
                            b7 ? Hmx::Color(1, 1, 0) : Hmx::Color(1, 0.64705884f, 0)
                        );
                    } else {
                        oneframe->AddSphere(
                            it->mInterest->WorldXfm().v,
                            2,
                            b7 ? Hmx::Color(1, 0, 0)
                               : Hmx::Color(0.6901961f, 0.1882353f, 0.3764706f)
                        );
                    }
                }
                if (it->IsInRefractoryPeriod()) {
                    oneframe->AddString3D(
                        MakeString("r=%f", it->RefractoryTimeRemaining()),
                        it->mInterest->WorldXfm().v,
                        Hmx::Color(1, 1, 1)
                    );
                }
            }
        }
    }
}

void CharEyes::Poll() {
    if (mEyes.empty()) {
        return;
    }
    RndTransformable *head = GetHead();
    if (!head) {
        return;
    }
    if (TheTaskMgr.DeltaSeconds() < 0) {
        Enter();
        return;
    }
    float f13 = 0;
    if (mCamWeight) {
        f13 = mCamWeight->Weight();
    }
    mLastLook += TheTaskMgr.DeltaSeconds();
    float f14 = mFaceServo ? mFaceServo->BlinkWeightLeft() : 0;
    bool b3 = false;
    if (f14 < 0.3f) {
        mBlinkDetect = true;
    } else if (mBlinkDetect && mLastBlinkWeight > 0.8f && f14 < mLastBlinkWeight) {
        mBlinkDetect = false;
        mBlinkWindowCount++;
        b3 = true;
        mLastBlinkDetectTime = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
    }
    mLastBlinkWeight = f14;

    const Transform &headXfm = head->WorldXfm();
    Vector3 ve0;
    Subtract(unk78, headXfm.v, ve0);
    Normalize(ve0, ve0);
    Vector3 vf8 = headXfm.m.y;
    Normalize(vf8, vf8);
    float clamped = Clamp(-1.0f, 1.0f, Dot(ve0, vf8));
    if (mLastCang != kHugeFloat) {
        TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
        mAvDelta = Interp(mAvDelta, clamped - mLastCang, 0.1f);
        float f9 = mCurInterest ? mCurInterest->MinLookTime() : 1;
        float f10 = mCurInterest ? mCurInterest->MaxLookTime() : 3;
        float f11 = mCurInterest ? mCurInterest->MaxViewAngleCos() : mMaxEyeCang;
        bool b11 = clamped >= f11;
        if (mLastLook <= f10 && !mNewFocusInterest) {
            if (mFocusInterest) {
                if (mCurInterest != mFocusInterest) {
                    if (mLastLook > 0.4f) {
                        if (mFocusInterest->IsWithinViewCone(headXfm.v, vf8)) {
                            goto lol;
                        }
                    }
                    if (IsHeadIKWeightIncreasing()) {
                        goto lol;
                    }
                }
            }
            if (mInterestFiltersChanged && mLastLook > 0.25f) {
                goto next;
            }
            if (mLastLook <= f9) {
                goto next;
            }
            if (b3 || !b11 || !EitherEyeClamped()) {
                goto next;
            }
            if (mAvDelta >= 0) {
                goto next;
            }
        }
    lol:
        if (f13 == 0) {
            NextLook();
        }
    }
next:
    mLastCang = clamped;
    mLastFacing = vf8;
    float f16 = 0;
    if (mHeadLookAt) {
        f16 = mHeadLookAt->Weight();
    }
    mLastHeadIKWeight = f16;
    DartUpdate();
    if (mCurInterest) {
        if (!mProceduralBlink) {
            unk78 = mCurInterest->WorldXfm().v;
        } else {
            mDelayedTarget = mCurInterest->WorldXfm().v;
        }
        EnforceMinimumTargetDistance(headXfm.v, unk78, unk78);
    }
    RndTransformable *target = GetTarget();
    if (target) {
        float wt = Weight();
        if (f13 > 0) {
            RndCam *cam = nullptr;
            if (TheWorld && TheWorld->Cam()) {
                cam = TheWorld->Cam();
            } else {
                cam = RndCam::Current();
                if (!RndCam::Current()) {
                    cam = TheRnd.GetDefaultCam();
                }
            }
            if (cam) {
                Transform xfm(target->WorldXfm());
                Interp(xfm.v, cam->WorldXfm().v, f13, xfm.v);
                target->SetWorldXfm(xfm);
            }
        } else {
            Vector3 vf82 = unk78;
            if (mDarting) {
                Add(vf82, mCurDartOffset, vf82);
                EnforceMinimumTargetDistance(headXfm.v, vf82, vf82);
            }
            Transform xfm(target->WorldXfm());
            Interp(xfm.v, vf82, wt, xfm.v);
            target->SetWorldXfm(xfm);
        }
    }
    ProceduralBlinkUpdate();
    CharLookAt::SetDisableJitter(sDisableEyeJitter);
    FOREACH (it, mEyes) {
        it->mEye->Poll();
        LidTrackAndClampingUpdate(*it, f14);
    }
    CharLookAt::SetDisableJitter(false);
    UpdateOverlay();
}

bool CharEyes::EitherEyeClamped() {
    FOREACH (it, mEyes) {
        if (it->mEye && it->mEye->Clamped()) {
            return true;
        }
    }
    return false;
}

void CharEyes::Enter() {
    mLastFacing.Zero();
    mBlinkDetect = false;
    mDarting = false;
    mDartsRemaining = -1;
    mLastLook = 0;
    mProceduralBlink = false;
    mAvDelta = 0;
    mBlinkWindowCount = 0;
    mLastCang = 1;
    mTargetTooClose = false;
    mLastBlinkWeight = -1;
    mDartNextEventTime = -1;
    mBlinkTimestamp = -1;
    mBlinkWindowSecsRemaining = -1;
    mLastBlinkDetectTime = -1;
    mInterestFilterFlags = mDefaultFilterFlags;
    mLastHeadIKWeight = 0;
    mInterestFiltersChanged = false;
    mNewFocusInterest = false;
    RndTransformable *head = GetHead();
    if (head) {
        mLastFacing = head->WorldXfm().m.y;
        Normalize(mLastFacing, mLastFacing);
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
    mFocusInterest = nullptr;
    mCurFocusPriorityClass = -1;
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
    if (mBlinksEnabled && !mProceduralBlink) {
        mProceduralBlink = true;
        mBlinkTimestamp = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
        mBlinkWindowCount++;
    }
}

void CharEyes::SetEnableBlinks(bool b1, bool b2) {
    mBlinksEnabled = b1;
    if (b2 && !b1 && mProceduralBlink && mFaceServo) {
        mFaceServo->SetProceduralBlinkWeight(0.0f);
        mProceduralBlink = false;
        unk78 = mDelayedTarget;
    }
}

bool CharEyes::SetFocusInterest(CharInterest *interest, int i) {
    if (mFocusInterest && mCurFocusPriorityClass > i) {
        return false;
    } else {
        mFocusInterest = interest;
        mCurFocusPriorityClass = i;
        if (mFocusInterest != interest) {
            mNewFocusInterest = true;
        }
        if (!mFocusInterest) {
            mCurFocusPriorityClass = -1;
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
        return (weight > 0 && weight - mLastHeadIKWeight > 0);
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
    if (mFocusInterest) {
        return mFocusInterest;
    } else if (mCurInterest) {
        return mCurInterest;
    } else {
        return nullptr;
    }
}

void CharEyes::ProceduralBlinkUpdate() {
    static DataNode &disableCheat = DataVariable("cheat.disable_procedural_blinks");

    if (!sDisableProceduralBlink && disableCheat.Int() == 0 && (mBlinksEnabled || mProceduralBlink)) {
        mBlinkWindowSecsRemaining = mBlinkWindowSecsRemaining - TheTaskMgr.DeltaSeconds();
        if (mBlinkWindowSecsRemaining < 0.0f) {
            mBlinkWindowCount = 0;
            mBlinkWindowSecsRemaining = 15.0f;
        }
        if (mFaceServo && mProceduralBlink) {
            float elapsed = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) - mBlinkTimestamp;
            if (elapsed < 0.115f) {
                // Closing phase
                float ease = EaseInExp(Clamp(0.0f, 1.0f, elapsed * 8.695652f));
                mFaceServo->SetProceduralBlinkWeight(ease);
            } else if (elapsed < 0.3f) {
                // Opening phase
                float t = Clamp(0.0f, 1.0f, 1.0f - (elapsed - 0.115f) * 5.4054055f);
                float ease = EaseSigmoid(t, 0, 0);
                mFaceServo->SetProceduralBlinkWeight(ease);
                unk78 = mDelayedTarget;
            } else {
                // Blink complete
                mFaceServo->SetProceduralBlinkWeight(0.0f);
                mProceduralBlink = false;
                unk78 = mDelayedTarget;
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
    mTargetTooClose = false;
    float f1;
    if (mCurInterest && mCurInterest->OverridesMinTargetDist()) {
        f1 = mCurInterest->MinTargetDistOverride();
    } else {
        f1 = mMinTargetDist;
    }
    if (len < f1) {
        Vector3 scaled;
        NormalizeScale(sub, f1, scaled);
        Add(v1, scaled, vres);
        mTargetTooClose = true;
    }
}

void CharEyes::DartUpdate() {
    static DataNode &n = DataVariable("cheat.disable_eye_darts");
    if (!sDisableEyeDart && n.Int() == 0) {
        mDartNextEventTime -= TheTaskMgr.DeltaSeconds();
        if (mDarting) {
            if (mDartNextEventTime < 0) {
                if (--mDartsRemaining < 0) {
                    mDarting = false;
                    mDartNextEventTime = RandomFloat(
                        mCurDartRuleset.mMinSecsBetweenSequences, mCurDartRuleset.mMaxSecsBetweenSequences
                    );
                } else {
                    mDartNextEventTime = RandomFloat(
                        mCurDartRuleset.mMinSecsBetweenDarts, mCurDartRuleset.mMaxSecsBetweenDarts
                    );
                    mCurDartOffset = GenerateDartOffset();
                }
            }
        } else {
            if (mDartNextEventTime < 0 && EyesOnTarget(mCurDartRuleset.mOnTargetAngleThresh) && !mProceduralBlink) {
                mDarting = true;
                mDartsRemaining =
                    RandomInt(mCurDartRuleset.mMinDartsPerSequence, mCurDartRuleset.mMaxDartsPerSequence);
                mDartNextEventTime =
                    RandomFloat(mCurDartRuleset.mMinSecsBetweenDarts, mCurDartRuleset.mMaxSecsBetweenDarts);
                mCurDartOffset = GenerateDartOffset();
            }
        }
    }
}

Vector3 CharEyes::GenerateDartOffset() {
    float min = mCurDartRuleset.mMinRadius;
    float max = mCurDartRuleset.mMaxRadius;
    if (mCurDartRuleset.mScaleWithDistance && mCurDartRuleset.mReferenceDistance > 0.1f) {
        Vector3 sub;
        Subtract(unk78, GetHead()->WorldXfm().v, sub);
        float scalar = Length(sub) / mCurDartRuleset.mReferenceDistance;
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
        if (mCurInterest) {
            if (mFocusInterest && streq(mCurInterest->Name(), mFocusInterest->Name())) {
                *mEyeStatusOverlay << "Look(FOC) ";
            } else {
                *mEyeStatusOverlay << "Look(" << mCurInterest->Name() << ") ";
            }
        } else {
            *mEyeStatusOverlay << "Look(GEN) ";
        }
        if (mFocusInterest) {
            const Transform &headXfm = GetHead()->WorldXfm();
            Vector3 v = headXfm.m.y;
            Normalize(v, v);
            const char *str = mFocusInterest->IsWithinViewCone(headXfm.v, v) ? "t" : "f";
            *mEyeStatusOverlay << "Foc(" << mFocusInterest->Name() << " p(" << mCurFocusPriorityClass << ") v("
                               << str << ")) ";
        } else {
            *mEyeStatusOverlay << "Foc(NA) ";
        }
        *mEyeStatusOverlay << "t(" << mLastLook << ") ";
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
        if (mProceduralBlink) {
            *mEyeStatusOverlay << "P Blink! ";
        }
        if (mDarting) {
            *mEyeStatusOverlay << "Dart! ";
        }
        if (mTargetTooClose) {
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
    if (mFocusInterest)
        SetFocusInterest(nullptr, 0);
    else
        SetFocusInterest(mCurInterest, 0);
    return 0;
}

DataNode CharEyes::OnToggleInterestOverlay(DataArray *) {
    ToggleInterestsDebugOverlay();
    return 0;
}
