#include "meta_ham/ChallengeResultPanel.h"
#include "HamPanel.h"
#include "flow/Flow.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/Challenges.h"
#include "net_ham/ChallengeSystemJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

ChallengeResultPanel::ChallengeResultPanel()
    : mList(0), mState(kStateInit), mScore(0), mMissionIndex(0), mCheckingIdx(0),
      mMyIndex(0) {}

ChallengeResultPanel::~ChallengeResultPanel() {}

BEGIN_HANDLERS(ChallengeResultPanel)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2)))
    HANDLE_MESSAGE(UIComponentScrollMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

BEGIN_PROPSYNCS(ChallengeResultPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void ChallengeResultPanel::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT_RANGE(data, 0, mItems.size(), 0x11a);
    static Symbol best_score("best_score");
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x11E);
    if (mItems[data].challengerUsername == gNullStr) {
        label->SetTextToken(gNullStr);
        return;
    } else {
        String curGamerTag = mItems[data].challengerUsername;
        if (slot->Matches("white_small_gamertag")) {
            if (mScore <= (int)mItems[data].score && data != mMissionIndex
                && data != mMyIndex) {
                label->SetPrelocalizedString(curGamerTag);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("grey_small_gamertag")) {
            if (mScore > (int)mItems[data].score && data != mMissionIndex) {
                label->SetPrelocalizedString(curGamerTag);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("white_large_gamertag")) {
            if (mScore <= (int)mItems[data].score && data == mMissionIndex) {
                label->SetPrelocalizedString(curGamerTag);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("grey_large_gamertag")) {
            if (mScore > (int)mItems[data].score && data == mMissionIndex) {
                label->SetPrelocalizedString(curGamerTag);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("gold_large_gamertag")) {
            if (mScore == (int)mItems[data].score && data == mMyIndex) {
                label->SetPrelocalizedString(curGamerTag);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("white_small_score")) {
            if (mScore <= (int)mItems[data].score && data != mMissionIndex
                && data != mMyIndex) {
                app_label->SetTokenFmt(
                    best_score, LocalizeSeparatedInt(mItems[data].score, TheLocale)
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("grey_small_score")) {
            if (mScore > (int)mItems[data].score && data != mMissionIndex) {
                app_label->SetTokenFmt(
                    best_score, LocalizeSeparatedInt(mItems[data].score, TheLocale)
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("white_large_score")) {
            if (mScore <= (int)mItems[data].score && data == mMissionIndex) {
                app_label->SetTokenFmt(
                    best_score, LocalizeSeparatedInt(mItems[data].score, TheLocale)
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("grey_large_score")) {
            if (mScore > (int)mItems[data].score && data == mMissionIndex) {
                app_label->SetTokenFmt(
                    best_score, LocalizeSeparatedInt(mItems[data].score, TheLocale)
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("gold_large_score")) {
            if (mScore == (int)mItems[data].score && data == mMyIndex) {
                app_label->SetTokenFmt(
                    best_score, LocalizeSeparatedInt(mItems[data].score, TheLocale)
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        }
    }
}

int ChallengeResultPanel::NumData() const { return mItems.size(); }

void ChallengeResultPanel::FinishLoad() {
    UIPanel::FinishLoad();
    mList = DataDir()->Find<UIList>("challengee.lst");
    mNavList = DataDir()->Find<HamNavList>("right_hand.hnl");
    mSettings = DataDir()->Find<PropertyEventProvider>("result.ep");
}

void ChallengeResultPanel::Poll() {
    HamPanel::Poll();
    switch (mState) {
    case kStateInit:
        if (!DataDir()->Find<Flow>("result_init.flow")->IsRunning()) {
            DataDir()->Find<Flow>("score.flow")->Activate();
            mState = kStateScrollToRival;
            mList->AutoScroll();
        }
        break;
    case kStateCheckingRivalResult:
        if (!DataDir()->Find<Flow>("rival_result.flow")->IsRunning()) {
            mState = kStateScrollToEnd;
            mList->AutoScroll();
        }
        break;
    case kStateScrollToEnd:
        if (!mList->IsScrolling()) {
            mList->StopAutoScroll();
            mState = kStateCheckingFinalResult;
            DataDir()->Find<Flow>("final_result.flow")->Activate();
        }
        break;
    case kStateCheckingFinalResult:
        if (!DataDir()->Find<Flow>("final_result.flow")->IsRunning()) {
            mState = kStateEnd;
            mNavList->Enable();
            mNavList->SetShowing(true);
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
    bool d16 = 0;
    String playerName;
    int numDisplay = mList->NumDisplay();
    int d15 = 0;
    int d14 = 0;
    int totalXP = TheChallenges->GetTotalXpEarned(player);
    HamPlayerData *playerData = TheGameData->Player(player);
    MILO_ASSERT(playerData, 0x7D);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x7F);
    mScore = provider->Property(score)->Int();
    mMissionIndex = provider->Property(challenge_mission_index)->Int() + numDisplay;
    unk68 = (SkeletonSide)provider->Property(side)->Int();
    playerName = provider->Property(player_name)->Str();
    int challengeScore = provider->Property(challenge_mission_score)->Int();
    bool challengeSelf = provider->Property(is_challenging_self)->Int();
    mCheckingIdx = (numDisplay / 2) + 1;
    if (mScore <= challengeScore) {
        mMissionIndex++;
    }
    mItems.clear();
    for (int i = 0; i < mList->NumDisplay(); i++) {
        mItems.push_back(ChallengeRow());
    }
    bool b3 = false;
    ChallengeRow row;
    row.score = mScore;
    row.challengerUsername = playerName;
    row.challengeeUsername = playerName;
    auto &challenges = TheChallenges->GetInGameChallenges(player);
    int numPlayerChallenges = challenges.size();
    for (int i = 0; i < numPlayerChallenges; i++) {
        if (mScore <= (int)challenges[i].score && !b3) {
            b3 = true;
            mMyIndex = mItems.size();
            mItems.push_back(row);
        }
        mItems.push_back(challenges[i]);
    }
    if (!b3) {
        mMyIndex = mItems.size();
        mItems.push_back(row);
    }
    int i8 = 0;
    int d20;
    for (int i = numDisplay; i < mItems.size(); i++) {
        if (mScore > (int)mItems[i].score) {
            if (i < mMissionIndex) {
                d15 +=
                    TheChallenges->CalculateChallengeXp(mItems[i].score, mItems[i].diff);
            } else if (i == mMissionIndex) {
                d14 = d15
                    + TheChallenges->CalculateChallengeXp(mItems[i].score, mItems[i].diff);
                d16 = 1;
            }
            i8++;
        }
    }
    if (i8 == 0) {
        d20 = 0;
    } else if (i8 == mItems.size() - numDisplay - 1) {
        d20 = 4;
    } else if (d16) {
        if (i8 > mMissionIndex + 1) {
            d20 = 3;
        } else {
            d20 = 2;
        }
    } else {
        d20 = 1;
    }
    mList->SetProperty(max_display, 0);
    mList->SetProperty(scroll_past_max_display, 1);
    mList->StopAutoScroll();
    mList->SetProvider(this);
    mNavList->Disable();
    mNavList->SetShowing(false);
    mSettings->SetProperty(rival_beaten, d16);
    mSettings->SetProperty(grade, d20);
    mSettings->SetProperty(side, unk68);
    mSettings->SetProperty(xp_before_mission, d15);
    mSettings->SetProperty(xp_mission, d14);
    mSettings->SetProperty(xp_total, totalXP);
    mSettings->SetProperty(rival_is_self, challengeSelf);
    mState = kStateInit;
    DataDir()->Find<Flow>("result_init.flow")->Activate();
}

DataNode ChallengeResultPanel::OnMsg(const UIComponentScrollMsg &msg) {
    if (msg.GetUIComponent() == mList) {
        int firstShowing = mList->FirstShowing();
        if (mMissionIndex - firstShowing == mCheckingIdx) {
            mList->StopAutoScroll();
            mState = kStateCheckingRivalResult;
            Flow *f = DataDir()->Find<Flow>("rival_result.flow", true);
            f->Activate();
        }
    }
    return 1;
}
