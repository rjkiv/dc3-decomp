#include "hamobj/HamScrollBehavior.h"
#include "HamListRibbon.h"
#include "HamNavProvider.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamNavList.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Task.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "ui/UI.h"
#include "ui/UIListProvider.h"
#include "ui/UIListState.h"
#include "ui/UIScreen.h"

float HamScrollBehavior::mNeutralToSlowDownDelay = 0;
float HamScrollBehavior::mSlowDownFirstTickDelay = 0;
float HamScrollBehavior::mSlowDownTickDelay = 0;
float HamScrollBehavior::mFastDownTickDelay = 0;
float HamScrollBehavior::mNeutralToSlowUpDelay = 0;
float HamScrollBehavior::mSlowUpFirstTickDelay = 0;
float HamScrollBehavior::mSlowUpTickDelay = 0;
float HamScrollBehavior::mFastUpTickDelay = 0;
float HamScrollBehavior::mSlowScrollSpeed = 0;
float HamScrollBehavior::mNormalScrollSpeed = 0;
float HamScrollBehavior::mFastScrollSpeedBase = 0;
float HamScrollBehavior::mFastScrollSpeedScalar = 0;
float HamScrollBehavior::mScrollUpCap = 0;
float HamScrollBehavior::mScrollDownCap = 0;
float HamScrollBehavior::mSlowFastThreshold = 0;
float HamScrollBehavior::sScrollSettleTime = 0.1f;

HamScrollBehavior::HamScrollBehavior(HamNavList *nav, UIListState *state)
    : mScrollingSettledTimer(0), mAtTop(false), mAtBottom(false), mScrollAmount(1),
      mScrollTimer(0), mScrollDelayBefore(0.3), mScrollDelayBetween(0),
      mScrollProgress(0), mOnScrollButton(false), mJustOnScrollButton(false),
      mScrollFrame(0), mScrollDir(kScrollDir_None), mBufferedScrollDir(kScrollDir_None),
      mLastScrollDir(kScrollDir_None), mHandScrollPos(kScrollDir_None),
      mScrollSpeedFrame(0, 10, 0), mScrollState(kScrollState_None), mListState(state),
      mOwner(nav) {}

void HamScrollBehavior::Init() {
    static Symbol ui("ui");
    static Symbol scroll_config("scroll_config");
    DataArray *uiCfg = SystemConfig(ui);
    if (uiCfg) {
        DataArray *cfg = uiCfg->FindArray(scroll_config, false);
        if (cfg) {
            static Symbol neutral_to_slow_down_delay("neutral_to_slow_down_delay");
            mNeutralToSlowDownDelay = cfg->FindFloat(neutral_to_slow_down_delay);
            static Symbol slow_down_first_tick_delay("slow_down_first_tick_delay");
            mSlowDownFirstTickDelay = cfg->FindFloat(slow_down_first_tick_delay);
            static Symbol slow_down_tick_delay("slow_down_tick_delay");
            mSlowDownTickDelay = cfg->FindFloat(slow_down_tick_delay);
            static Symbol fast_down_tick_delay("fast_down_tick_delay");
            mFastDownTickDelay = cfg->FindFloat(fast_down_tick_delay);
            static Symbol neutral_to_slow_up_delay("neutral_to_slow_up_delay");
            mNeutralToSlowUpDelay = cfg->FindFloat(neutral_to_slow_up_delay);
            static Symbol slow_up_first_tick_delay("slow_up_first_tick_delay");
            mSlowUpFirstTickDelay = cfg->FindFloat(slow_up_first_tick_delay);
            static Symbol slow_up_tick_delay("slow_up_tick_delay");
            mSlowUpTickDelay = cfg->FindFloat(slow_up_tick_delay);
            static Symbol fast_up_tick_delay("fast_up_tick_delay");
            mFastUpTickDelay = cfg->FindFloat(fast_up_tick_delay);
            static Symbol slow_scroll_speed("slow_scroll_speed");
            mSlowScrollSpeed = cfg->FindFloat(slow_scroll_speed);
            static Symbol normal_scroll_speed("normal_scroll_speed");
            mNormalScrollSpeed = cfg->FindFloat(normal_scroll_speed);
            static Symbol fast_scroll_speed_base("fast_scroll_speed_base");
            mFastScrollSpeedBase = cfg->FindFloat(fast_scroll_speed_base);
            static Symbol fast_scroll_speed_scalar("fast_scroll_speed_scalar");
            mFastScrollSpeedScalar = cfg->FindFloat(fast_scroll_speed_scalar);
            static Symbol scroll_up_cap("scroll_up_cap");
            mScrollUpCap = cfg->FindFloat(scroll_up_cap);
            static Symbol scroll_down_cap("scroll_down_cap");
            mScrollDownCap = cfg->FindFloat(scroll_down_cap);
            static Symbol slow_fast_threshold("slow_fast_threshold");
            mSlowFastThreshold = cfg->FindFloat(slow_fast_threshold);
        }
    }
}

bool HamScrollBehavior::ScrollUp(bool interrupt) {
    if (mScrollProgress > 0.0f && !interrupt)
        return false;
    int i = mListState->FirstShowing() - mScrollAmount;
    if (i < 0)
        return false;
    mListState->Scroll(-1, false);
    mListState->Poll(0.0f);
    mOwner->HandleHighlightChanged(i);
    mScrollDir = kScrollDir_Up;
    mScrollingSettledTimer = sScrollSettleTime;
    return true;
}

bool HamScrollBehavior::ScrollDown(bool interrupt) {
    if (mScrollProgress > 0.0f && !interrupt)
        return false;
    int i2 = mListState->FirstShowing() + mScrollAmount
        + HamListRibbon::sNumListSelectable - 1;
    if (i2 - mScrollAmount >= mListState->NumShowing())
        return false;
    mOwner->HandleHighlightChanged(i2);
    mScrollDir = kScrollDir_Down;
    mScrollingSettledTimer = sScrollSettleTime;
    return true;
}

bool HamScrollBehavior::IsScrolling() const {
    return mScrollDir != kScrollDir_None
        || (mHandScrollPos == kScrollDir_Up && mListState->FirstShowing() != 0)
        || (mHandScrollPos == kScrollDir_Down && !AtBottom());
}

bool HamScrollBehavior::AtTop() const { return mListState->FirstShowing() == 0; }

bool HamScrollBehavior::AtBottom() const {
    return mListState->FirstShowing()
        == mListState->NumShowing() - HamListRibbon::sNumListSelectable;
}

void HamScrollBehavior::Enter() {
    mOwner->SetScrollSoundFrame(0);
    mOwner->PlayScrollSound();
}

void HamScrollBehavior::Exit() {
    Reset();
    mOwner->StopScrollSound();
}

void HamScrollBehavior::Reset() {
    mHandScrollPos = kScrollDir_None;
    mLastScrollDir = kScrollDir_None;
    mBufferedScrollDir = kScrollDir_None;
    mScrollingSettledTimer = 0;
    mScrollDir = kScrollDir_None;
    mScrollSpeedFrame.ForceValue(0);
    mOwner->SetScrollSoundFrame(mScrollSpeedFrame.Value());
    mScrollTimer = 0;
    mAtTop = false;
    mScrollFrame = 0;
    mAtBottom = false;
    mScrollProgress = 0;
    mScrollState = kScrollState_None;
}

void HamScrollBehavior::Update(float handHeight) {
    ListScrollDirection handScrollPos = mHandScrollPos;
    if (mScrollingSettledTimer > 0 && mScrollDir == kScrollDir_None) {
        if (mAtBottom || mAtTop) {
            mScrollingSettledTimer = 0;
        }
        mScrollingSettledTimer = mScrollingSettledTimer - TheTaskMgr.DeltaUISeconds();
        bool check = mScrollingSettledTimer <= 0;
        if (check) {
            static Message scrollingSettledMsg("scrolling_settled");
            if (TheUI->CurrentScreen()) {
                TheUI->CurrentScreen()->Handle(scrollingSettledMsg, false);
            }
            if (mOwner) {
                mOwner->SendHighlightSettledMsg(mListState->SelectedData());
            }
        }
    }
    if (handScrollPos != kScrollDir_None) {
        mLastScrollDir = handScrollPos;
        float neutralToSlowDownDelay;
        if (handScrollPos == kScrollDir_Up) {
            neutralToSlowDownDelay = mNeutralToSlowUpDelay;
        } else {
            neutralToSlowDownDelay = mNeutralToSlowDownDelay;
        }
        mScrollTimer += TheTaskMgr.DeltaUISeconds();
        if ((!mOnScrollButton && mScrollTimer >= neutralToSlowDownDelay)
            || (mOnScrollButton && mScrollTimer >= mScrollDelayBetween)) {
            if (!mOnScrollButton) {
                mJustOnScrollButton = true;
            } else {
                mJustOnScrollButton = false;
            }
            mScrollTimer = 0;
            mOnScrollButton = true;
            if (handScrollPos == kScrollDir_Up) {
                ScrollUp(false);
            } else if (handScrollPos == kScrollDir_Down) {
                ScrollDown(false);
            }
        }
    } else {
        mOnScrollButton = false;
        mJustOnScrollButton = false;
        mScrollTimer = 0;
    }

    float f7;
    if (handHeight > 0.5f) {
        f7 = Clamp(0.0f, 1.0f, (handHeight - 1) / mScrollDownCap);
    } else {
        f7 = Clamp(-1.0f, 0.0f, (handHeight / mScrollUpCap));
    }
    f7 = fabsf(f7);

    float f9;
    float normalScrollSpeed;

    if (TheGestureMgr && !TheGestureMgr->InControllerMode() && mOnScrollButton) {
        if (mScrollTimer <= 0.001f) {
            if (f7 < mSlowFastThreshold || mScrollState == kScrollState_None) {
                normalScrollSpeed = mSlowScrollSpeed;
                if (mJustOnScrollButton) {
                    mScrollDelayBetween = handScrollPos == kScrollDir_Up
                        ? mSlowUpFirstTickDelay
                        : mSlowDownFirstTickDelay;
                } else {
                    mScrollDelayBetween = handScrollPos == kScrollDir_Up
                        ? mSlowUpTickDelay
                        : mSlowDownTickDelay;
                }
                f9 = 0;
                mScrollState =
                    handScrollPos == kScrollDir_Up ? kScrollState_Up : kScrollState_Down;
            } else {
                normalScrollSpeed = mFastScrollSpeedScalar * f7 + mFastScrollSpeedBase;
                mScrollDelayBetween = handScrollPos == kScrollDir_Up ? mFastUpTickDelay
                                                                     : mFastDownTickDelay;
                f9 = (f7 - mSlowFastThreshold) / (1 - mSlowFastThreshold);
                mScrollState = handScrollPos == kScrollDir_Up ? kScrollState_FastUp
                                                              : kScrollState_FastDown;
            }
        } else {
            switch (mScrollState) {
            case 0:
            case 4:
                normalScrollSpeed = mFastScrollSpeedScalar * f7 + mFastScrollSpeedBase;
                f9 = (f7 - mSlowFastThreshold) / (1 - mSlowFastThreshold);
                break;
            case 1:
            case 3:
                normalScrollSpeed = mSlowScrollSpeed;
                f9 = 0;
                break;
            default:
                normalScrollSpeed = 0;
                f9 = 0;
                break;
            }
        }
    } else {
        f9 = 0;
        normalScrollSpeed = mNormalScrollSpeed;
        mScrollDelayBetween = 0;
        mScrollState = kScrollState_None;
    }

    mScrollSpeedFrame.Smooth(f9, TheTaskMgr.DeltaUISeconds());
    mOwner->SetScrollSoundFrame(mScrollSpeedFrame.Value());

    RndAnimatable *anim = mOwner->GetScrollSoundAnim();
    if (anim) {
        switch (mScrollState) {
        case 0:
            anim->SetFrame(-1 - mScrollSpeedFrame.Value(), 1);
            break;
        case 1:
            anim->SetFrame(-1, 1);
            break;
        case 2:
            if (handHeight > 0.5f) {
                if (AtBottom()
                    || mOwner->GetRibbonMode() == HamListRibbon::kRibbonDisengaged) {
                    anim->SetFrame(0, 1);
                } else {
                    anim->SetFrame(mOwner->CalculateSwell(5), 1);
                }
            } else {
                if (mListState->FirstShowing() == 0) {
                    anim->SetFrame(0, 1);
                } else {
                    anim->SetFrame(-mOwner->CalculateSwell(0), 1);
                }
            }
            break;
        case 3:
            anim->SetFrame(1, 1);
            break;
        case 4:
            anim->SetFrame(mScrollSpeedFrame.Value() + 1, 1);
            break;
        default:
            break;
        }
    }

    if (mScrollDir) {
        mScrollProgress =
            TheTaskMgr.DeltaUISeconds() * normalScrollSpeed + mScrollProgress;
        if (mScrollProgress > 1) {
            mScrollFrame = 0;
            if (mScrollDir == kScrollDir_Down) {
                mListState->SetSelected(
                    mListState->FirstShowing() + HamListRibbon::sNumListSelectable - 1,
                    mListState->FirstShowing(),
                    true
                );
                mListState->Scroll(1, false);
                mListState->Poll(0);
            }
            bool b5 = false;
            if (mBufferedScrollDir == mScrollDir) {
                mScrollProgress -= 1;
                if (mScrollDir == kScrollDir_Down) {
                    b5 = ScrollDown(true);
                } else {
                    b5 = ScrollUp(true);
                }
            }

            if (b5) {
                if (mScrollDir == kScrollDir_Up) {
                    mScrollFrame = 1 - mScrollProgress;
                } else {
                    mScrollFrame = mScrollProgress;
                }
                mBufferedScrollDir = kScrollDir_None;
            } else {
                mScrollProgress = 0;
                mScrollDir = kScrollDir_None;
                mBufferedScrollDir = kScrollDir_None;
            }
            return;
        }
        if (mScrollDir == kScrollDir_Up) {
            mScrollFrame = 1 - mScrollProgress;
        } else {
            mScrollFrame = mScrollProgress;
        }
    }
    mBufferedScrollDir = kScrollDir_None;
}
