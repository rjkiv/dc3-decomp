#include "ui/UITrigger.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/EventTrigger.h"
#include "ui/UIComponent.h"
#include "utl/Loader.h"

UITrigger::UITrigger()
    : mBlockTransition(0), mCallbackObject(this), mEndTime(0), unk13c(1) {}

BEGIN_PROPSYNCS(UITrigger)
    SYNC_PROP(block_transition, mBlockTransition)
    SYNC_PROP(callback_object, mCallbackObject)
    SYNC_SUPERCLASS(EventTrigger)
END_PROPSYNCS

BEGIN_SAVES(UITrigger)
    bs << 1;
    SAVE_SUPERCLASS(EventTrigger)
    bs << mBlockTransition;
END_SAVES

BEGIN_COPYS(UITrigger)
    COPY_SUPERCLASS(EventTrigger)
    CREATE_COPY(UITrigger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBlockTransition)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(UITrigger)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    if (d.rev < 1) {
        UIComponent *uiCom = Hmx::Object::New<UIComponent>();
        uiCom->Load(bs);
        delete uiCom;
        Symbol sym;
        bs >> sym;
        UnregisterEvents();
        mTriggerEvents.clear();
        mTriggerEvents.push_back(sym);
        RegisterEvents();
        ObjPtr<RndAnimatable> animPtr(this);
        bs >> animPtr;
        mAnims.clear();
        mAnims.push_back();
        EventTrigger::Anim &anim = mAnims.back();
        anim.mAnim = animPtr;
    } else
        LOAD_SUPERCLASS(EventTrigger);
    bs >> mBlockTransition;
END_LOADS

void UITrigger::Trigger() {}

DataArray *UITrigger::SupportedEvents() {
    static DataArray *events =
        SystemConfig("objects", "UITrigger", "supported_events")->Array(1);
    return events;
}

void UITrigger::CheckAnims() {
    FOREACH (it, mAnims) {
        Anim &curAnim = *it;
        RndAnimatable *anim = curAnim.mAnim;
        if (anim && anim->GetRate() != RndAnimatable::k30_fps_ui) {
            if (TheLoadMgr.EditMode()) {
                MILO_NOTIFY("Setting animatable rate to k30_fps_ui for %s", anim->Name());
            }
            anim->SetRate(RndAnimatable::k30_fps_ui);
        }
        curAnim.mRate = RndAnimatable::k30_fps_ui;
    }
}

void UITrigger::Poll() {
    if (!unk13c) {
        if (IsDone()) {
            unk13c = true;
            if (mCallbackObject) {
                mCallbackObject->Handle(UITriggerCompleteMsg(this), true);
            }
        }
    }
}

void UITrigger::Enter() {
    mStartTime = TheTaskMgr.UISeconds();
    mEndTime = 0;
}

bool UITrigger::IsDone() const { return mEndTime <= TheTaskMgr.UISeconds(); }

bool UITrigger::IsBlocking() const {
    if (mStartTime > TheTaskMgr.UISeconds()) {
        const_cast<UITrigger *>(this)->mEndTime = 0;
    }
    return mBlockTransition && mEndTime && !IsDone();
}

void UITrigger::StopAnimations() {
    FOREACH (it, mAnims) {
        RndAnimatable *anim = (*it).mAnim;
        if (anim && anim->IsAnimating())
            anim->StopAnimation();
    }
}

void UITrigger::PlayStartOfAnims() {
    FOREACH (it, mAnims) {
        Anim &curAnim = *it;
        RndAnimatable *anim = curAnim.mAnim;
        if (anim) {
            float f3 = anim->StartFrame();
            float f4 = 0.0099999998f;
            if (curAnim.mEnable) {
                f3 = curAnim.mStart;
                if (f3 > curAnim.mEnd) {
                    f4 *= -1;
                }
            }
            anim->Animate(f3 + f4, f3, kTaskUISeconds, 0, 0, 0, kEaseLinear, 0, 0);
        }
    }
}

void UITrigger::PlayEndOfAnims() {
    FOREACH (it, mAnims) {
        Anim &curAnim = *it;
        RndAnimatable *anim = curAnim.mAnim;
        if (anim) {
            float f3 = anim->EndFrame();
            float f4 = 0.0099999998f;
            if (curAnim.mEnable) {
                f3 = curAnim.mEnd;
                if (curAnim.mStart > f3) {
                    f4 *= -1;
                }
            }
            anim->Animate(f3 - f4, f3, kTaskUISeconds, 0, 0, 0, kEaseLinear, 0, 0);
        }
    }
}

BEGIN_HANDLERS(UITrigger)
    HANDLE_EXPR(end_time, mEndTime)
    HANDLE_ACTION(play_start_of_anims, PlayStartOfAnims())
    HANDLE_ACTION(play_end_of_anims, PlayEndOfAnims())
    HANDLE_ACTION(stop_anims, StopAnimations())
    HANDLE_EXPR(is_done, IsDone())
    HANDLE_EXPR(is_blocking, IsBlocking())
    HANDLE_SUPERCLASS(EventTrigger)
END_HANDLERS
