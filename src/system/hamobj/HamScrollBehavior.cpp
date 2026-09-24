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
    : unk0(0), unk4(0), unk5(0), unk8(1), unkc(0), unk10(0.3), unk14(0), unk18(0),
      unk1c(0), unk1d(0), unk20(0), unk24(0), unk28(0), unk2c(0), unk30(0),
      unk34(0, 10, 0), unk48(2), unk4c(state), unk50(nav) {}

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

bool HamScrollBehavior::ScrollUp(bool b) {
    if (unk18 > 0.0f && !b)
        return false;
    int i = unk4c->FirstShowing() - unk8;
    if (i < 0)
        return false;
    unk4c->Scroll(-1, false);
    unk4c->Poll(0.0f);
    unk50->HandleHighlightChanged(i);
    unk24 = 1;
    unk0 = sScrollSettleTime;
    return true;
}

bool HamScrollBehavior::ScrollDown(bool b1) {
    if (unk18 > 0.0f && !b1)
        return false;
    int i2 = unk4c->FirstShowing() + unk8 + HamListRibbon::sNumListSelectable - 1;
    if (i2 - unk8 >= unk4c->NumShowing())
        return false;
    unk50->HandleHighlightChanged(i2);
    unk24 = 2;
    unk0 = sScrollSettleTime;
    return true;
}

bool HamScrollBehavior::IsScrolling() const {
    return unk24 != 0 || (unk30 == 1 && unk4c->FirstShowing() != 0)
        || (unk30 == 2 && !AtBottom());
}

bool HamScrollBehavior::AtTop() const { return unk4c->FirstShowing() == 0; }

bool HamScrollBehavior::AtBottom() const {
    return unk4c->FirstShowing()
        == unk4c->NumShowing() - HamListRibbon::sNumListSelectable;
}

void HamScrollBehavior::Enter() {
    unk50->SetScrollSoundFrame(0);
    unk50->PlayScrollSound();
}

void HamScrollBehavior::Exit() {
    Reset();
    unk50->StopScrollSound();
}

void HamScrollBehavior::Reset() {
    unk30 = 0;
    unk2c = 0;
    unk28 = 0;
    unk0 = 0;
    unk24 = 0;
    unk34.Reset();
    unk50->SetScrollSoundFrame(unk34.Level());
    unkc = 0;
    unk4 = false;
    unk20 = 0;
    unk5 = false;
    unk18 = 0;
    unk48 = 2;
}

void HamScrollBehavior::Update(float f1) {
    int i28 = unk30;
    if (unk0 > 0 && unk24 == 0) {
        if (unk5 || unk4) {
            unk0 = 0;
        }
        unk0 = unk0 - TheTaskMgr.DeltaUISeconds();
        bool check = unk0 <= 0;
        if (check) {
            static Message scrollingSettledMsg("scrolling_settled");
            if (TheUI->CurrentScreen()) {
                TheUI->CurrentScreen()->Handle(scrollingSettledMsg, false);
            }
            if (unk50) {
                unk50->SendHighlightSettledMsg(unk4c->SelectedData());
            }
        }
    }
    if (i28 != 0) {
        unk2c = i28;
        float f16;
        if (i28 == 1) {
            f16 = mNeutralToSlowUpDelay;
        } else {
            f16 = mNeutralToSlowDownDelay;
        }
        unkc += TheTaskMgr.DeltaUISeconds();
        if ((!unk1c && unkc >= f16) || (unk1c && unkc >= unk14)) {
            if (!unk1c) {
                unk1d = true;
            } else {
                unk1d = false;
            }
            unkc = 0;
            unk1c = true;
            if (i28 == 1) {
                ScrollUp(false);
            } else if (i28 == 2) {
                ScrollDown(false);
            }
        }
    } else {
        unk1c = false;
        unk1d = false;
        unkc = 0;
    }

    float f7;
    if (f1 > 0.5f) {
        f7 = Clamp(0.0f, 1.0f, (f1 - 1) / mScrollDownCap);
    } else {
        f7 = Clamp(-1.0f, 0.0f, (f1 / mScrollUpCap));
    }
    f7 = fabsf(f7);

    float f9;
    float f10;

    if (TheGestureMgr && !TheGestureMgr->InControllerMode() && unk1c) {
        if (unkc <= 0.001f) {
            if (f7 < mSlowFastThreshold || unk48 == 2) {
                f10 = mSlowScrollSpeed;
                if (unk1d) {
                    unk14 = i28 == 1 ? mSlowUpFirstTickDelay : mSlowDownFirstTickDelay;
                } else {
                    unk14 = i28 == 1 ? mSlowUpTickDelay : mSlowDownTickDelay;
                }
                f9 = 0;
                unk48 = i28 == 1 ? 1 : 3;
            } else {
                f10 = mFastScrollSpeedScalar * f7 + mFastScrollSpeedBase;
                unk14 = i28 == 1 ? mFastUpTickDelay : mFastDownTickDelay;
                f9 = (f7 - mSlowFastThreshold) / (1 - mSlowFastThreshold);
                unk48 = i28 == 1 ? 0 : 4;
            }
        } else {
            switch (unk48) {
            case 0:
            case 4:
                f10 = mFastScrollSpeedScalar * f7 + mFastScrollSpeedBase;
                f9 = (f7 - mSlowFastThreshold) / (1 - mSlowFastThreshold);
                break;
            case 1:
            case 3:
                f10 = mSlowScrollSpeed;
                f9 = 0;
                break;
            default:
                f10 = 0;
                f9 = 0;
                break;
            }
        }
    } else {
        f9 = 0;
        f10 = mNormalScrollSpeed;
        unk14 = 0;
        unk48 = 2;
    }

    unk34.Smooth(f9, TheTaskMgr.DeltaUISeconds());
    unk50->SetScrollSoundFrame(unk34.Level());

    RndAnimatable *anim = unk50->GetScrollSoundAnim();
    if (anim) {
        switch (unk48) {
        case 0:
            anim->SetFrame(-1 - unk34.Level(), 1);
            break;
        case 1:
            anim->SetFrame(-1, 1);
            break;
        case 2:
            if (f1 > 0.5f) {
                if (AtBottom()
                    || unk50->GetRibbonMode() == HamListRibbon::kRibbonDisengaged) {
                    anim->SetFrame(0, 1);
                } else {
                    anim->SetFrame(unk50->CalculateSwell(5), 1);
                }
            } else {
                if (unk4c->FirstShowing() == 0) {
                    anim->SetFrame(0, 1);
                } else {
                    anim->SetFrame(-unk50->CalculateSwell(0), 1);
                }
            }
            break;
        case 3:
            anim->SetFrame(1, 1);
            break;
        case 4:
            anim->SetFrame(unk34.Level() + 1, 1);
            break;
        default:
            break;
        }
    }

    if (unk24) {
        unk18 = TheTaskMgr.DeltaUISeconds() * f10 + unk18;
        if (unk18 > 1) {
            unk20 = 0;
            if (unk24 == 2) {
                unk4c->SetSelected(
                    unk4c->FirstShowing() + HamListRibbon::sNumListSelectable - 1,
                    unk4c->FirstShowing(),
                    true
                );
                unk4c->Scroll(1, false);
                unk4c->Poll(0);
            }
            bool b5 = false;
            if (unk28 == unk24) {
                unk18 -= 1;
                if (unk24 == 2) {
                    b5 = ScrollDown(true);
                } else {
                    b5 = ScrollUp(true);
                }
            }

            if (b5) {
                if (unk24 == 1) {
                    unk20 = 1 - unk18;
                } else {
                    unk20 = unk18;
                }
                unk28 = 0;
            } else {
                unk18 = 0;
                unk24 = 0;
                unk28 = 0;
            }
            return;
        }
        if (unk24 == 1) {
            unk20 = 1 - unk18;
        } else {
            unk20 = unk18;
        }
    }
    unk28 = 0;
}
