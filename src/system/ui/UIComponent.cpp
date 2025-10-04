#include "ui/UIComponent.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"

Symbol UIComponentStateToSym(UIComponent::State s) {
    static Symbol syms[5] = { "normal", "focused", "disabled", "selecting", "selected" };
    return syms[s];
}

Symbol UIComponent::StateSym() const {
    return UIComponentStateToSym((UIComponent::State)mState);
}

void UIComponent::Enter() {
    RndPollable::Enter();
    mSelected = 0;
    if (mState == kSelecting) {
        SetState(kFocused);
    }
}

UIComponent::UIComponent()
    : mState(kNormal), mNavRight(this), mNavDown(this), mSelectScreen(nullptr),
      mSelected(0), unk40(0) {}

BEGIN_PROPSYNCS(UIComponent)
    SYNC_PROP(nav_right, mNavRight)
    SYNC_PROP(nav_down, mNavDown)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndPollable)
END_PROPSYNCS

void UIComponent::PostLoad(BinStream &) {}
