#include "ui/UIComponent.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "ui/UI.h"
#include "utl/BinStream.h"

int UIComponent::sSelectFrames = 0;

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

BEGIN_COPYS(UIComponent)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY_AS(UIComponent, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mNavRight)
        COPY_MEMBER(mNavDown)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_SAVES(UIComponent)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mNavRight;
    bs << mNavDown;
END_SAVES

BEGIN_LOADS(UIComponent)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

UIComponent::State SymToUIComponentState(Symbol s) {
    for (int i = 0; i < 5; i++) {
        if (s.Str() == UIComponentStateToSym((UIComponent::State)i).Str())
            return (UIComponent::State)i;
    }
    MILO_ASSERT(false, 0x22);
    return UIComponent::kNumStates;
}

void UIComponent::SetState(UIComponent::State s) {
    if (!CanHaveFocus() && s == kFocused) {
        MILO_NOTIFY(
            "Component: %s cannot have focus.  Why are we setting it to the focused state?",
            Name()
        );
        s = kNormal;
    }
    mState = s;
}

void UIComponent::OldResourcePreload(BinStream &bs) {
    char c[264];
    bs.ReadString(c, 0x100);
}

void UIComponent::Init() {
    REGISTER_OBJ_FACTORY(UIComponent);
    sSelectFrames = SystemConfig("objects", "UIComponent")->FindInt("select_frames");
}

void UIComponent::Poll() {
    if (mSelected == 0)
        return;
    if (--mSelected != 0)
        return;
    FinishSelecting();
}

void UIComponent::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(3, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    LOAD_SUPERCLASS(RndDrawable)
    if (d.rev > 0) {
        bs >> mNavRight;
        bs >> mNavDown;
    }
    if (1 < d.rev && d.rev < 3) {
        // call somethin related to bs
    }
}

void UIComponent::SendSelect(LocalUser *user) {
    if (mState == kFocused) {
        SetState(kSelecting);
        static UIComponentSelectMsg select_msg(0, 0);
        select_msg[0] = DataNode(this);
        select_msg[1] = DataNode(user);
        TheUI->Handle(select_msg, false);
        if (mState != kSelecting)
            mSelectScreen = 0;
        else {
            mSelectScreen = TheUI->CurrentScreen();
            mSelectingUser = user;
            mSelected = sSelectFrames;
        }
    }
}

void UIComponent::FinishSelecting() {
    if (mState != kDisabled && mState != kNormal)
        SetState(kFocused);
    if (!unk40 && mSelectScreen == TheUI->CurrentScreen()) {
        static UIComponentSelectDoneMsg select_msg(this, 0);
        select_msg[0] = DataNode(this);
        select_msg[1] = DataNode(mSelectingUser);
        TheUI->Handle(select_msg, false);
    } else
        unk40 = false;
}

BEGIN_HANDLERS(UIComponent)
    HANDLE_EXPR(get_state, GetState())
    HANDLE_ACTION(set_state, SetState((UIComponent::State)_msg->Int(2)))
    HANDLE_EXPR(can_have_focus, CanHaveFocus())
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
