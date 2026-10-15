#include "hamobj/HamScrollSpeedIndicator.h"
#include "math/Easing.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "utl/BinStream.h"

HamScrollSpeedIndicator::HamScrollSpeedIndicator()
    : mIsShowing(0), mEnterAnim(this), mExitAnim(this), mIndicatorAnim(this) {}

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
    FOREACH (it, mDraws) {
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
    mIsShowing = enter;
}

void HamScrollSpeedIndicator::Draw(const Transform &worldXfm) {
    SetWorldXfm(worldXfm);
    FOREACH (it, mDraws) {
        (*it)->Draw();
    }
}

INIT_REVS(2, 0)

void HamScrollSpeedIndicator::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0)
    RndDir::PreLoad(bs);
    PUSH_REVS(d);
}

void HamScrollSpeedIndicator::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    RndDir::PostLoad(bs);
    if (d.rev >= 1) {
        d >> mEnterAnim;
        d >> mExitAnim;
        d >> mIndicatorAnim;
    }
    if (d.rev >= 2) {
        d >> mSlowScrollThresholdFrame;
        d >> mFastScrollThresholdFrame;
    }
}

void HamScrollSpeedIndicator::Update(
    float handHeight, float scrollUpCap, float scrollDownCap
) {
    float range = (mIndicatorAnim->EndFrame() - mIndicatorAnim->StartFrame()) / 2;
    float f9;

    if (handHeight >= 0.1f && handHeight <= 0.9f) {
        f9 = (handHeight - 0.1f) * 1.25f;
        f9 = -(f9 * mSlowScrollThresholdFrame * 2 - mSlowScrollThresholdFrame);
    } else {
        if (handHeight > 0.5f) {
            float fvar3 = range - mSlowScrollThresholdFrame;
            handHeight -= 0.9f;
            scrollDownCap = scrollDownCap + 1 - 0.9f;
            f9 = -(handHeight / scrollDownCap);
            f9 = f9 * fvar3 - mSlowScrollThresholdFrame;
        } else {
            float fvar3 = range - mSlowScrollThresholdFrame;
            handHeight -= 0.1f;
            scrollUpCap += 0.1f;
            f9 = -(handHeight / scrollUpCap);
            f9 = f9 * fvar3 + mSlowScrollThresholdFrame;
        }
    }
    if (mIsShowing) {
        float frame = Clamp(mIndicatorAnim->StartFrame(), mIndicatorAnim->EndFrame(), f9);
        mIndicatorAnim->SetFrame(frame, 1);
    }
}
