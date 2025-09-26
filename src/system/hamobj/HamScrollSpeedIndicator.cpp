#include "hamobj/HamScrollSpeedIndicator.h"
#include "math/Easing.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"

HamScrollSpeedIndicator::HamScrollSpeedIndicator()
    : unk1fc(0), mEnterAnim(this), mExitAnim(this), mIndicatorAnim(this) {}

BEGIN_HANDLERS(HamScrollSpeedIndicator)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_PROPSYNCS(HamScrollSpeedIndicator)
    SYNC_PROP(enter_anim, mEnterAnim)
    SYNC_PROP(exit_anim, mExitAnim)
    SYNC_PROP(indicator_anim, mIndicatorAnim)
    SYNC_PROP(slow_scroll_threshold_frame, mSlowScrollThresholdFrame)
    SYNC_PROP(fast_scroll_threshold_frame, mFastScrollThresholdFrame)
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(HamScrollSpeedIndicator)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(RndDir)
    bs << mEnterAnim;
    bs << mExitAnim;
    bs << mIndicatorAnim;
    bs << mSlowScrollThresholdFrame;
    bs << mFastScrollThresholdFrame;
END_SAVES

BEGIN_COPYS(HamScrollSpeedIndicator)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY(HamScrollSpeedIndicator)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mEnterAnim)
        COPY_MEMBER(mExitAnim)
        COPY_MEMBER(mIndicatorAnim)
        COPY_MEMBER(mSlowScrollThresholdFrame)
        COPY_MEMBER(mFastScrollThresholdFrame)
    END_COPYING_MEMBERS
END_COPYS

void HamScrollSpeedIndicator::DrawShowing() {
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        (*it)->Draw();
    }
}

float HamScrollSpeedIndicator::StartFrame() {
    if (mEnterAnim)
        return mEnterAnim->StartFrame();
    else
        return 0;
}

float HamScrollSpeedIndicator::EndFrame() {
    if (mEnterAnim)
        return mEnterAnim->EndFrame();
    else
        return 0;
}

void HamScrollSpeedIndicator::HandleEnter() { Show(true); }
void HamScrollSpeedIndicator::HandleExit() { Show(false); }

void HamScrollSpeedIndicator::Show(bool enter) {
    if (enter) {
        mEnterAnim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    } else
        mExitAnim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    unk1fc = enter;
}

void HamScrollSpeedIndicator::Draw(const Transform &xfm) {
    SetWorldXfm(xfm);
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        (*it)->Draw();
    }
}
