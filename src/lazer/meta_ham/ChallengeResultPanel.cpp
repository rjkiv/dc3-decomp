#include "meta_ham/ChallengeResultPanel.h"
#include "HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/Challenges.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

ChallengeResultPanel::ChallengeResultPanel()
    : mChallengeList(0), unk4c(0), unk5c(0), unk60(0), unk64(0), unk6c(0) {}

ChallengeResultPanel::~ChallengeResultPanel() {}

BEGIN_HANDLERS(ChallengeResultPanel)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2)))
    HANDLE_MESSAGE(UIComponentScrollMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

BEGIN_PROPSYNCS(ChallengeResultPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void ChallengeResultPanel::Text(int, int data, UIListLabel *, UILabel *label) const {
    MILO_ASSERT_RANGE(data, 0, mItems.size(), 0x11a);
    static Symbol best_score("best_score");
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x11E);
}

int ChallengeResultPanel::NumData() const { return mItems.size(); }

void ChallengeResultPanel::FinishLoad() {
    UIPanel::FinishLoad();
    mChallengeList = DataDir()->Find<UIList>("challengee.lst");
    mRightHandNavList = DataDir()->Find<HamNavList>("right_hand.hnl");
    mResultEventProvider = DataDir()->Find<PropertyEventProvider>("result.ep");
}

void ChallengeResultPanel::Poll() {
    HamPanel::Poll();
    switch (unk4c) {
    case 0:
        if (!DataDir()->Find<Flow>("result_init.flow")->IsRunning()) {
            DataDir()->Find<Flow>("score.flow")->Activate();
            unk4c = 1;
            mChallengeList->AutoScroll();
        }
        break;
    case 2:
        if (!DataDir()->Find<Flow>("rival_result.flow")->IsRunning()) {
            unk4c = 3;
            mChallengeList->AutoScroll();
        }
        break;
    case 3:
        if (!mChallengeList->IsScrolling()) {
            mChallengeList->StopAutoScroll();
            unk4c = 4;
            DataDir()->Find<Flow>("final_result.flow")->Activate();
        }
        break;
    case 4:
        if (!DataDir()->Find<Flow>("final_result.flow")->IsRunning()) {
            unk4c = 5;
            mRightHandNavList->Enable();
            mRightHandNavList->SetShowing(true);
        }
        break;
    default:
        break;
    }
}

void ChallengeResultPanel::UpdateList(int player) {
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
    String str180;
    int numDisplay = mChallengeList->NumDisplay();
    int totalXP = TheChallenges->GetTotalXpEarned(player);
    HamPlayerData *playerData = TheGameData->Player(player);
    MILO_ASSERT(playerData, 0x7D);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x7F);
    unk5c = provider->Property(score)->Int();
    unk60 = provider->Property(challenge_mission_index)->Int() + numDisplay;
    unk68 = provider->Property(side)->Int();
    str180 = provider->Property(player_name)->Str();
    int challengeScore = provider->Property(challenge_mission_score)->Int();
    bool challengeSelf = provider->Property(is_challenging_self)->Int();
    unk64 = (numDisplay + 1) % 2; // unsure about this part
    if (unk5c <= challengeScore) {
        unk60++;
    }
    mItems.clear();
    for (int i = 0; i < mChallengeList->NumDisplay(); i++) {
        mItems.push_back(ChallengeRow());
    }
    // more
    DataDir()->Find<Flow>("result_init.flow")->Activate();
}
