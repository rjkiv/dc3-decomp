#include "meta_ham/ChallengeResultPanel.h"
#include "HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamNavList.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIList.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

ChallengeRow::ChallengeRow() {}

ChallengeResultPanel::ChallengeResultPanel()
    : unk40(), unk4c(0), unk5c(0), unk60(0), unk64(0), unk6c(0) {}

ChallengeResultPanel::~ChallengeResultPanel() {}

BEGIN_PROPSYNCS(ChallengeResultPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

int ChallengeResultPanel::NumData() const { return mItems.size(); }

void ChallengeResultPanel::FinishLoad() {
    UIPanel::FinishLoad();
    unk40 = DataDir()->Find<UIList>("challengee.lst", true);
    unk44 = DataDir()->Find<HamNavList>("right_hand.hnl", true);
    unk48 = DataDir()->Find<PropertyEventProvider>("result.ep", true);
}

void ChallengeResultPanel::Poll() { HamPanel::Poll(); }

void ChallengeResultPanel::Text(int, int data, UIListLabel *, UILabel *) const {
    MILO_ASSERT_RANGE(data, 0, mItems.size(), 0x11a);
    static Symbol best_score("best_score");
    // need AppPanel
}

void ChallengeResultPanel::UpdateList(int) {
    static Symbol score("score");
    static Symbol challenge_mission_index("challenge_mission_index");
    static Symbol side("side");
    static Symbol scroll_past_max_display("scroll_past_max_display");
    static Symbol max_display("max_display");
    static Symbol rival_beaten("rival_beaten");
    static Symbol grade("grade");
    static Symbol player_name("player_name");
    static Symbol challenge_mission_score("challenge_mission_score");
    static Symbol xp_before_mission("xp_before_mission");
    static Symbol xp_mission("xp_mission");
    static Symbol xp_total("xp_total");
    static Symbol is_challenging_self("is_challenging_self");
    static Symbol rival_is_self("rival_is_self");
}

BEGIN_HANDLERS(ChallengeResultPanel)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
