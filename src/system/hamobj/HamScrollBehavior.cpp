#include "hamobj/HamScrollBehavior.h"
#include "HamListRibbon.h"
#include "hamobj/HamNavList.h"
#include "obj/Data.h"
#include "os/System.h"
#include "ui/UIListState.h"

float HamScrollBehavior::sScrollSettleTime = 0.1;

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

bool HamScrollBehavior::ScrollDown(bool b1) {
    if (unk18 > 0 && !b1)
        return false;
    else {
        int i2 = unk4c->FirstShowing() + unk8 + 3;
        if (i2 - unk8 < unk4c->NumShowing()) {
            unk50->HandleHighlightChanged(i2);
            unk24 = 2;
            unk0 = sScrollSettleTime;
            return true;
        }
    }
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
