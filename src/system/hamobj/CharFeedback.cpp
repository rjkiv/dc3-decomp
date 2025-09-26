#include "hamobj/CharFeedback.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"

bool CharFeedback::sEnabled = true;

CharFeedback::CharFeedback()
    : mTarget(this), mFailMat(this), mFailTriggerSecs(0.1), mMinFailSecs(0.2),
      mFadeSecs(1), mTestLimbs(kFeedbackNone) {
    Sync();
}

CharFeedback::~CharFeedback() {}

BEGIN_HANDLERS(CharFeedback)
    HANDLE_ACTION(trigger_fail, TestUpdateLimbs(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharFeedback)
    SYNC_PROP_MODIFY(target, mTarget, Sync())
    SYNC_PROP_MODIFY(fail_mat, mFailMat, Sync())
    SYNC_PROP(fail_trigger_secs, mFailTriggerSecs)
    SYNC_PROP(min_fail_secs, mMinFailSecs)
    SYNC_PROP_SET(test_limbs, (int &)mTestLimbs, mTestLimbs = (FeedbackLimbs)_val.Int())
    SYNC_PROP(fade_secs, mFadeSecs)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharFeedback)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mTarget;
    bs << mTestLimbs;
    bs << mFailTriggerSecs;
    bs << mMinFailSecs;
    bs << mFailMat;
    bs << mFadeSecs;
END_SAVES

BEGIN_COPYS(CharFeedback)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(CharFeedback)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mTestLimbs)
        COPY_MEMBER(mFailTriggerSecs)
        COPY_MEMBER(mMinFailSecs)
        COPY_MEMBER(mFailMat)
        COPY_MEMBER(mFadeSecs)
    END_COPYING_MEMBERS
END_COPYS

void CharFeedback::Enter() {
    RndPollable::Enter();
    Sync();
}

void CharFeedback::TestUpdateLimbs(bool b1) {
    Sync();
    for (int i = 0; i < kNumLimbFeedbacks; i++) {
        if (mTestLimbs & (1 << i)) {
            UpdateLimb(i, b1);
        }
    }
}

void CharFeedback::ResetErrors() {
    for (int i = 0; i < kNumLimbFeedbacks; i++) {
        mLimbStates[i].unk1 = 0;
        mLimbStates[i].unk0 = 0;
        mLimbStates[i].unk4 = -1;
        mLimbStates[i].unk8 = 0;
    }
}

void CharFeedback::UpdateLimb(int limb_index, bool b2) {
    MILO_ASSERT((0) <= (limb_index) && (limb_index) < (kNumLimbFeedbacks), 0x25);
    LimbState &cur = mLimbStates[limb_index];
    if (b2 != cur.unk1) {
        float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime);
        if (b2 || cur.unk4 == -1 || secs - cur.unk4 > mMinFailSecs) {
            cur.unk4 = secs;
            cur.unk1 = b2;
        }
    }
}

void CharFeedback::Sync() {
    static const char *sLimbMeshes[4] = {
        "left_arm.mesh", "right_arm.mesh", "left_leg.mesh", "right_leg.mesh"
    };
    for (int i = 0; i < kNumLimbFeedbacks; i++) {
        mLimbStates[i].unkc = nullptr;
    }
    ResetErrors();
    if (mTarget) {
        for (int i = 0; i < kNumLimbFeedbacks; i++) {
            RndMesh *mesh = mTarget->Find<RndMesh>(sLimbMeshes[i], false);
            mLimbStates[i].unkc = mesh;
            if (mesh) {
                mesh->SetShowing(false);
                mesh->SetMat(mFailMat);
            }
        }
    }
}
