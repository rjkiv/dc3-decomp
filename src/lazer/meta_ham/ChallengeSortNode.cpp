#include "ChallengeSortNode.h"

#include "Accomplishment.h"
#include "AppLabel.h"
#include "ChallengeSortMgr.h"
#include "Challenges.h"
#include "HamStarsDisplay.h"
#include "ProfileMgr.h"
#include "hamobj/HamGameData.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/NavListNode.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "lazer/net_ham/RockCentral.h"
#include "ui/UILabel.h"
#include "ui/UIListCustom.h"
#include "ui/UIListLabel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region ChallengeHeaderNode
ChallengeHeaderNode::ChallengeHeaderNode(NavListItemSortCmp *cmp, Symbol sym, bool b)
    : NavListHeaderNode(cmp, sym, b), mChallengeCount(0) {}

int ChallengeHeaderNode::GetChallengeExp() {
    FOREACH (it, Children()) {
        auto node = *it;
        MILO_ASSERT(node, 0xd0);
        // TheChallenges->CalculateChallengeXp()
    }
    return 0;
}

NavListSortNode *ChallengeHeaderNode::GetFirstActive() {
    FOREACH (it, mChildren) {
        auto node = (*it)->GetFirstActive();
        if (node)
            break;
    }
    auto something = TheChallengeSortMgr->NumData();
    if (something)
        return this;
    return nullptr;
}

void ChallengeHeaderNode::Renumber(std::vector<NavListSortNode *> &vec) {
    mStartIx = vec.size();
    if (TheChallengeSortMgr->HeadersSelectable()) {
        vec.push_back(this);
        TheChallengeSortMgr->AddHeaderIndex(mStartIx);
    }
    if (!TheChallengeSortMgr->IsHeaderCollapsed(GetToken())) {
        FOREACH (it, mChildren) {
            (*it)->Renumber(vec);
        }
    }
}

void ChallengeHeaderNode::Text(UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x91);
    if (uiListLabel->Matches("sort_header")) {
        app_label->SetFromGeneralSelectNode(this);
    } else if (uiListLabel->Matches("challenge_count")) {
        // something
    } else if (uiListLabel->Matches("header_collapse")) {
    }
}

void ChallengeHeaderNode::OnHighlight() {
    SongPreview *preview = TheChallengeSortMgr->GetSongPreview();
    preview->Start(0, 0);
    SetCollapseStateIcon(true);
}

Symbol ChallengeHeaderNode::OnSelect() {
    if (TheChallengeSortMgr->IsHeaderCollapsed(GetToken())) {
        TheChallengeSortMgr->SetHeaderUncollapsed(GetToken());
    } else {
        TheChallengeSortMgr->SetHeaderCollapsed(GetToken());
    }
    return gNullStr;
}

void ChallengeHeaderNode::SetCollapseStateIcon(bool b) const {
    Symbol stateIcon = gNullStr;
    UILabel *iconLabel = GetCollapseIconLabel();
    if (iconLabel) {
        static Symbol header_open_icon("header_open_icon");
        static Symbol header_open_highlighted_icon("header_open_highlighted_icon");
        static Symbol header_closed_icon("header_closed_icon");
        static Symbol header_closed_highlighted_icon("header_closed_highlighted_icon");
        if (TheChallengeSortMgr->IsHeaderCollapsed(GetToken())) {
            if (b) {
                stateIcon = header_closed_highlighted_icon;
            } else {
                stateIcon = header_closed_icon;
            }
        } else {
            if (b) {
                stateIcon = header_open_highlighted_icon;
            } else {
                stateIcon = header_open_icon;
            }
        }
        iconLabel->SetTextToken(stateIcon);
    }
}

Symbol ChallengeHeaderNode::OnSelectDone() {
    if (TheChallengeSortMgr->IsInHeaderMode()
        && !TheChallengeSortMgr->EnteringHeaderMode()) {
        TheChallengeSortMgr->SetHeaderMode(false);
    }

    TheChallengeSortMgr->OnEnter();
    TheChallengeSortMgr->GetCurrentSort()->BuildItemList();
    return gNullStr;
}

BEGIN_HANDLERS(ChallengeHeaderNode)
    HANDLE_EXPR(get_challenge_count, mChallengeCount)
    HANDLE_SUPERCLASS(NavListHeaderNode)
END_HANDLERS

#pragma endregion

#pragma region ChallengeSortNode

int ChallengeSortNode::GetChallengeExp() {
    return TheChallenges->CalculateChallengeXp(
        mChallengeRecord->GetChallengeRow().mDiff,
        mChallengeRecord->GetChallengeRow().mType
    );
}

int ChallengeSortNode::GetSongID() { return mChallengeRecord->GetChallengeRow().mSongID; }

int ChallengeSortNode::GetChallengeScore() {
    return mChallengeRecord->GetChallengeRow().mScore;
}

int ChallengeSortNode::GetChallengerXp() {
    return mChallengeRecord->GetChallengeRow().mChallengerXp;
}

const char *ChallengeSortNode::GetChallengerGamertag() {
    int type = mChallengeRecord->GetChallengeRow().mType;
    bool flag;
    if (type < 0 || type > 2) {
        flag = false;
    }
    if (!flag) {
        if (type < 3 || type > 5) {
            flag = false;
        }
        if (!flag) {
            return mChallengeRecord->GetUnk48().Str();
        }
    }
    return "HARMONIX";
}

void ChallengeSortNode::SetMedalIcon(UILabel *label) const {
    MILO_ASSERT(label, 0x2bc);
    static Symbol challenge_gold_icon("challenge_gold_icon");
    static Symbol challenge_silver_icon("challenge_silver_icon");
    static Symbol challenge_bronze_icon("challenge_bronze_icon");
    Symbol ret(gNullStr);
    int type = mChallengeRecord->GetChallengeRow().mType;
    switch (type) {
    case 0:
        ret = challenge_gold_icon;
        break;
    case 1:
        ret = challenge_silver_icon;
        break;
    case 2:
        ret = challenge_bronze_icon;
        break;
    case 3:
        ret = challenge_gold_icon;
        break;
    case 4:
        ret = challenge_silver_icon;
        break;
    case 5:
        ret = challenge_bronze_icon;
        break;
    default:
        break;
    }
    label->SetTextToken(ret);
}

void ChallengeSortNode::SetNewIcon(UILabel *label) const {
    MILO_ASSERT(label, 0x2da);
    AppLabel *appLabel = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(appLabel, 0x2dc);
    int timestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
        mChallengeRecord->GetChallengeRow().mSongID
    );
    if (timestamp > mChallengeRecord->GetChallengeRow().mTimeStamp
        || mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
        || mChallengeRecord->GetUnk50() == 4 || mChallengeRecord->GetUnk50() == 2
        || mChallengeRecord->GetUnk50() == 3) {
        appLabel->SetNew(false);
    } else {
        appLabel->SetNew(true);
    }
}

void ChallengeSortNode::SetBuyIcon(UILabel *label) const {
    MILO_ASSERT(label, 0x2f4);
    AppLabel *appLabel = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(appLabel, 0x2f6);
    if (mChallengeRecord->GetUnk50() == 4 || mChallengeRecord->GetUnk50() == 2
        || mChallengeRecord->GetUnk50() == 3) {
        appLabel->SetNew(true);
    } else {
        appLabel->SetNew(false);
    }
}

int ChallengeSortNode::GetPlayerSide() const {
    static Symbol ui_nav_player("ui_nav_player");
    static Symbol side("side");
    auto playerData =
        TheGameData->Player(TheHamProvider->Property(ui_nav_player, true)->Int());
    MILO_ASSERT(playerData, 0x316);
    auto provider = playerData->Provider();
    MILO_ASSERT(provider, 0x319);
    return provider->Property(side, true)->Int();
}

Symbol ChallengeSortNode::GetToken() const { return mChallengeRecord->GetUnk40(); }

void ChallengeSortNode::Text(UIListLabel *listlabel, UILabel *label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x1e5);
    if (listlabel->Matches("gamertag")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );
        if (mChallengeRecord->GetChallengeRow().mScore <= ownerChallengeScore) {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (mChallengeRecord->GetChallengeRow().mTimeStamp
                < ownerChallengeTimestamp) {
                app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
            } else if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()) {
                app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
            }
        }
    }
    if (listlabel->Matches("low_gamertag")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );
        if (mChallengeRecord->GetChallengeRow().mScore <= ownerChallengeScore
            && TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                   mChallengeRecord->GetChallengeRow().mSongID
               ) > mChallengeRecord->GetChallengeRow().mTimeStamp) {
            app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
        }
    }
    if (listlabel->Matches("right_gamertag")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 0) {
            app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
        }
    }
    if (listlabel->Matches("left_gamertag")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && !GetPlayerSide()) {
            app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
        }
    }
    if (listlabel->Matches("score")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );
        if (mChallengeRecord->GetChallengeRow().mScore < ownerChallengeScore) {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (ownerChallengeTimestamp
                <= mChallengeRecord->GetChallengeRow().mTimeStamp) {
                app_label->SetChallengeScoreLabel(
                    mChallengeRecord->GetChallengeRow().mScore
                );
            } else if (mChallengeRecord->GetUnk48() != mChallengeRecord->GetUnk4c()) {
                app_label->SetChallengeScoreLabel(
                    mChallengeRecord->GetChallengeRow().mScore
                );
            }
        }
    } else if (listlabel->Matches("low_score")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );
        if (mChallengeRecord->GetChallengeRow().mScore < ownerChallengeScore
            && TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                   mChallengeRecord->GetChallengeRow().mSongID
               ) > mChallengeRecord->GetChallengeRow().mTimeStamp) {
            app_label->SetChallengeScoreLabel(mChallengeRecord->GetChallengeRow().mScore);
        }
    } else if (listlabel->Matches("right_score")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 1) {
            app_label->SetChallengeScoreLabel(mChallengeRecord->GetChallengeRow().mScore);
        }
    } else if (listlabel->Matches("left_score")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && !GetPlayerSide()) {
            app_label->SetChallengeScoreLabel(mChallengeRecord->GetChallengeRow().mScore);
        }
    } else {
        if (listlabel->Matches("medal")) {
            SetMedalIcon(label);
        }
        if (listlabel->Matches("new")) {
            SetNewIcon(label);
        }
        if (listlabel->Matches("buy")) {
            SetBuyIcon(label);
        }
        listlabel->Matches("header_collapse");
    }
    Symbol blank(gNullStr);
    label->SetTextToken(blank);
}

Symbol ChallengeSortNode::Select() {
    static Symbol locked_content_screen("locked_content_screen");
    static Symbol store_loading_screen("store_loading_screen");
    static Symbol show_offers_need_to_sign_in_screen("show_offers_need_to_sign_in_screen");
    static Symbol server_not_available_screen("server_not_available_screen");
    Symbol screen = show_offers_need_to_sign_in_screen;
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
    activeProfile->UpdateOnlineID();
    if (activeProfile && activeProfile->GetOnlineID()) {
        if (ThePlatformMgr.IsSignedIntoLive(activeProfile->GetPadNum())
            && TheRockCentral.IsOnline()) {
            screen = store_loading_screen;
        }
    }
    static Symbol should_back_to_challenges("should_back_to_challenges");
    if (mChallengeRecord->GetUnk50() == 1) {
        MILO_ASSERT("false", 0x1a9);
    }
    return 0;
}

Symbol ChallengeSortNode::OnSelect() {
    if (UseQuickplayPerformer()) {
        MetaPerformer::Current()->ResetSongs();
    }
    Symbol sel = Select();
    if (sel != gNullStr) {
        auto obj = ObjectDir::Main()->Find<UIScreen>(sel.Str(), true);
        TheUI->GotoScreen(obj, false, false);
        return gNullStr;
    } else {
        return TheChallengeSortMgr->MoveOn();
    }
}

void ChallengeSortNode::OnContentMounted(const char *contentName, const char *c2) {
    MILO_ASSERT(contentName, 0x1c1);
    if (!TheContentMgr.RefreshInProgress()) {
        int songID = mChallengeRecord->GetChallengeRow().mSongID;
        Symbol sContentName(contentName);
        if (TheHamSongMgr.IsContentUsedForSong(sContentName, songID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message msg(song_data_mounted);
            TheUI->Handle(msg, false);
        }
    }
}

void ChallengeSortNode::Custom(UIListCustom *list, Hmx::Object *obj) const {
    if (list->Matches("stars")) {
        HamStarsDisplay *pStarsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarsDisplay, 0x294);
        pStarsDisplay->SetShowing(true);
    }
}

#pragma endregion
