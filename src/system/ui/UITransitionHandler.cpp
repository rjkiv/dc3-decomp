#include "ui/UITransitionHandler.h"
#include "math/Easing.h"
#include "rndobj/Anim.h"
#include "ui/UI.h"
#include "utl/BinStream.h"

UITransitionHandler::UITransitionHandler(Hmx::Object *obj)
    : mInAnim(obj), mOutAnim(obj), mAnimationState(kUITransitionAnimationInvalid),
      mChangePending(0), unk31(0) {}

UITransitionHandler::~UITransitionHandler() {
    if (mInAnim)
        mInAnim->StopAnimation();
    if (mOutAnim)
        mOutAnim->StopAnimation();
}

void UITransitionHandler::SetInAnim(RndAnimatable *anim) { mInAnim = anim; }

void UITransitionHandler::SetOutAnim(RndAnimatable *anim) { mOutAnim = anim; }

RndAnimatable *UITransitionHandler::GetInAnim() const { return mInAnim; }

RndAnimatable *UITransitionHandler::GetOutAnim() const { return mOutAnim; }

void UITransitionHandler::FinishValueChange() {
    if (IsEmptyValue())
        ClearAnimationState();
    else {
        if (mOutAnim && !TheUI->InTransition()) {
            unk31 = true;
            mOutAnim->Animate(0.0f, false, 0.0f, 0, kEaseLinear, 0, 0);
            mAnimationState = kUITransitionAnimationOutAnimating;
        } else
            mAnimationState = kUITransitionAnimationIdle;
    }
    mChangePending = false;
}

void UITransitionHandler::StartValueChange() {
    mChangePending = true;
    if (mAnimationState == 0) {
        FinishValueChange();
    } else if (mAnimationState == 1) {
        if (mInAnim && !TheUI->InTransition()) {
            mInAnim->Animate(0.0f, false, 0.0f, 0, kEaseLinear, 0, 0);
            mAnimationState = kUITransitionAnimationInAnimating;
        } else {
            FinishValueChange();
        }
    } else if (mAnimationState == 3) {
        MILO_ASSERT(mOutAnim, 0x89);
        if (unk31)
            FinishValueChange();
        else {
            mOutAnim->Animate(
                mOutAnim->Frame(),
                mOutAnim->StartFrame(),
                mOutAnim->Units(),
                0.0f,
                0.0f,
                0,
                kEaseLinear,
                0,
                0
            );
            mAnimationState = kUITransitionAnimationReverseOutAnimating;
        }
    }
}

void UITransitionHandler::UpdateHandler() {
    unk31 = false;
    if (mChangePending && IsReadyToChange()) {
        FinishValueChange();
    }
    if (mAnimationState == 3) {
        MILO_ASSERT(mOutAnim, 0x41);
        if (!mOutAnim->IsAnimating())
            mAnimationState = kUITransitionAnimationIdle;
    }
}

void UITransitionHandler::SaveHandlerData(BinStream &bs) { bs << mInAnim << mOutAnim; }

void UITransitionHandler::CopyHandlerData(const UITransitionHandler *ui) {
    mInAnim = ui->mInAnim;
    mOutAnim = ui->mOutAnim;
}

bool UITransitionHandler::HasTransitions() const {
    return (mInAnim.operator->() || mOutAnim.operator->());
}

bool UITransitionHandler::IsReadyToChange() const {
    bool ret = false;
    if (mAnimationState == kUITransitionAnimationIdle
        || mAnimationState == kUITransitionAnimationInvalid) {
        ret = true;
    } else if (mAnimationState == kUITransitionAnimationInAnimating) {
        MILO_ASSERT(mInAnim, 0x57);
        ret = !mInAnim->IsAnimating();
    } else if (mAnimationState == kUITransitionAnimationReverseOutAnimating) {
        MILO_ASSERT(mOutAnim, 0x5e);
        ret = !mOutAnim->IsAnimating();
    } else {
        MILO_ASSERT(false, 0x66);
    }
    return ret;
}

void UITransitionHandler::LoadHandlerData(BinStream &bs) { bs >> mInAnim >> mOutAnim; }

void UITransitionHandler::ClearAnimationState() {
    mAnimationState = kUITransitionAnimationInvalid;
}
