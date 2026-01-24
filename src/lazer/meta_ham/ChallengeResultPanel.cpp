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
    String playerName;
    int numDisplay = mChallengeList->NumDisplay();
    int totalXP = TheChallenges->GetTotalXpEarned(player);
    HamPlayerData *playerData = TheGameData->Player(player);
    MILO_ASSERT(playerData, 0x7D);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x7F);
    unk5c = provider->Property(score)->Int();
    unk60 = provider->Property(challenge_mission_index)->Int() + numDisplay;
    unk68 = provider->Property(side)->Int();
    playerName = provider->Property(player_name)->Str();
    int challengeScore = provider->Property(challenge_mission_score)->Int();
    bool challengeSelf = provider->Property(is_challenging_self)->Int();
    unk64 = (numDisplay / 2) + 1;
    if (unk5c <= challengeScore) {
        unk60++;
    }
    mItems.clear();
    for (int i = 0; i < mChallengeList->NumDisplay(); i++) {
        mItems.push_back(ChallengeRow());
    }

    // Create temp ChallengeRow with player's info
    ChallengeRow playerRow;
    playerRow.mScore = unk5c;
    playerRow.mGamertag = playerName;
    playerRow.unk2c = playerName;

    // Get player challenges from TheChallenges
    std::vector<ChallengeRow> &challenges = TheChallenges->mPlayerChallenges[player];
    bool inserted = false;

    // Insert player row and challenges in sorted order by score
    for (int n = challenges.size(); n > 0; n--) {
        int i = challenges.size() - n;
        if (unk5c <= challenges[i].mScore && !inserted) {
            unk6c = mItems.size();
            mItems.push_back(playerRow);
            inserted = true;
        }
        mItems.push_back(challenges[i]);
    }

    // If player wasn't inserted yet, insert at end
    if (!inserted) {
        unk6c = mItems.size();
        mItems.push_back(playerRow);
    }

    // Calculate XP values
    int xpBefore = 0;
    int xpMission = 0;
    bool beatRival = false;
    int beatenCount = 0;

    for (unsigned int i = numDisplay; i < mItems.size(); i++) {
        if (unk5c > mItems[i].mScore) {
            if (i < (unsigned int)unk60) {
                xpBefore += TheChallenges->CalculateChallengeXp(
                    mItems[i].mScore, mItems[i].mDiff
                );
            } else if (i == (unsigned int)unk60) {
                xpMission = xpBefore + TheChallenges->CalculateChallengeXp(
                    mItems[i].mScore, mItems[i].mDiff
                );
                beatRival = true;
            }
            beatenCount++;
        }
    }

    // Calculate grade (0-4)
    int gradeValue;
    if (beatenCount == 0) {
        gradeValue = 0;
    } else if ((unsigned int)beatenCount == mItems.size() - numDisplay - 1) {
        gradeValue = 4;
    } else if (beatRival) {
        if (beatenCount > unk60 + 1) {
            gradeValue = 3;
        } else {
            gradeValue = 2;
        }
    } else {
        gradeValue = 1;
    }

    // Set properties on UIList
    mChallengeList->SetProperty(max_display, DataNode(0));
    mChallengeList->SetProperty(scroll_past_max_display, DataNode(1));
    mChallengeList->StopAutoScroll();
    mChallengeList->SetProvider(this);

    // Disable and hide right hand nav list
    mRightHandNavList->Disable();
    mRightHandNavList->SetShowing(false);

    // Set properties on mResultEventProvider
    mResultEventProvider->SetProperty(rival_beaten, DataNode((int)beatRival));
    mResultEventProvider->SetProperty(grade, DataNode(gradeValue));
    mResultEventProvider->SetProperty(side, DataNode((int)unk68));
    mResultEventProvider->SetProperty(xp_before_mission, DataNode(xpBefore));
    mResultEventProvider->SetProperty(xp_mission, DataNode(xpMission));
    mResultEventProvider->SetProperty(xp_total, DataNode(totalXP));
    mResultEventProvider->SetProperty(rival_is_self, DataNode((int)challengeSelf));

    unk4c = 0;
    DataDir()->Find<Flow>("result_init.flow")->Activate();
}
