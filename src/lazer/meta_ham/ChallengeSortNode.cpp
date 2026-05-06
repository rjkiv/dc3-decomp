#include "ChallengeSortNode.h"

#include "Accomplishment.h"
#include "AppLabel.h"
#include "ChallengeSortMgr.h"
#include "Challenges.h"
#include "HamStarsDisplay.h"
#include "ProfileMgr.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/ChallengeRecord.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "lazer/net_ham/RockCentral.h"
#include "ui/UILabel.h"
#include "ui/UIListCustom.h"
#include "ui/UIListLabel.h"
#include "ui/UIScreen.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region ChallengeHeaderNode
ChallengeHeaderNode::ChallengeHeaderNode(NavListItemSortCmp *cmp, Symbol sym, bool b)
    : NavListHeaderNode(cmp, sym, b), mChallengeCount(0) {}

int ChallengeHeaderNode::GetChallengeExp() {
    int xp = 0;
    FOREACH (it, Children()) {
        ChallengeSortNode *node = static_cast<ChallengeSortNode *>(*it);
        MILO_ASSERT(node, 0xd0);
        xp += TheChallenges->CalculateChallengeXp(
            node->GetChallengeScore(), node->GetDifficulty()
        );
    }
    return xp;
}

int ChallengeHeaderNode::GetPotentialChallengeExp(NavListSortNode *n) {
    auto it = mChildren.begin();
    for (; it != mChildren.end() && *it != n; ++it) {
    }
    int xp = 0;
    for (; it != mChildren.end(); ++it) {
        ChallengeSortNode *node = static_cast<ChallengeSortNode *>(*it);
        MILO_ASSERT(node, 0xe7);
        xp += TheChallenges->CalculateChallengeXp(
            node->GetChallengeScore(), node->GetDifficulty()
        );
    }
    return xp;
}

NavListSortNode *ChallengeHeaderNode::GetFirstActive() {
    FOREACH (it, mChildren) {
        auto node = (*it)->GetFirstActive();
        if (node)
            return TheChallengeSortMgr->HeadersSelectable() ? this : node;
    }
    return nullptr;
}

void ChallengeHeaderNode::Renumber(std::vector<NavListSortNode *> &vec) {
    mStartIx = vec.size();
    if (TheChallengeSortMgr->GetHeadersSelectable()) {
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
        SetItemCountString(uiLabel);
    } else if (uiListLabel->Matches("header_collapse")) {
        if (TheChallengeSortMgr->GetHighlightItem() == this) {
            SetCollapseStateIcon(true);
        } else {
            SetCollapseStateIcon(false);
        }
    } else {
        uiLabel->SetTextToken(gNullStr);
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

char const *ChallengeHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");

    if (TheChallengeSortMgr->GetCurrentSort()->GetSortName() == by_album
        && GetToken() != singles) {
        auto node = mChildren.begin();
        if (node != mChildren.end())
            return (*node)->GetAlbumArtPath();
    }
    return 0;
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

bool ChallengeHeaderNode::IsActive() const {
    return TheChallengeSortMgr->HeadersSelectable() != false;
}

int ChallengeHeaderNode::GetSongID() {
    if (mChildren.size() == 0) {
        return 0;
    }
    ChallengeSortNode *node = static_cast<ChallengeSortNode *>(mChildren.front());
    MILO_ASSERT(node, 0x136);
    return node->GetChallengeRecord()->GetChallengeRow().mSongID;
}

Symbol ChallengeHeaderNode::GetSongShortName() {
    if (mChildren.size() == 0) {
        return gNullStr;
    }
    return mChildren.front()->GetToken();
}

String ChallengeHeaderNode::GetSongShortTitle() {
    if (mChildren.size() == 0) {
        return gNullStr;
    }

    ChallengeSortNode *node = static_cast<ChallengeSortNode *>(mChildren.front());
    MILO_ASSERT(node, 0x149);
    return node->GetChallengeRecord()->GetUnk44().Str();
}

int ChallengeHeaderNode::GetTotalEarnedExp(int score) {
    int xp = 0;
    FOREACH (it, mChildren) {
        ChallengeSortNode *node = static_cast<ChallengeSortNode *>(*it);
        MILO_ASSERT(node, 0xf5);
        if (score >= node->GetChallengeRecord()->GetChallengeRow().mScore) {
            xp += TheChallenges->CalculateChallengeXp(
                node->GetChallengeScore(), node->GetDifficulty()
            );
        }
    }
    if (xp == 0) {
        xp = TheChallenges->ConsolationXP();
    }
    return xp;
}

BEGIN_HANDLERS(ChallengeHeaderNode)
    HANDLE_EXPR(get_challenge_count, mChallengeCount)
    HANDLE_SUPERCLASS(NavListHeaderNode)
END_HANDLERS

#pragma endregion

#pragma region ChallengeSortNode

int ChallengeSortNode::GetChallengeExp() {
    ChallengeRecord *record = mChallengeRecord;
    return TheChallenges->CalculateChallengeXp(
        record->GetChallengeRow().mScore, record->GetChallengeRow().mDiff
    );
}

int ChallengeSortNode::GetSongID() { return mChallengeRecord->GetChallengeRow().mSongID; }

int ChallengeSortNode::GetChallengeScore() {
    return mChallengeRecord->GetChallengeRow().mScore;
}

int ChallengeSortNode::GetChallengerXp() {
    return mChallengeRecord->GetChallengeRow().mChallengerXp;
}

int ChallengeSortNode::GetDifficulty() {
    return mChallengeRecord->GetChallengeRow().mDiff;
}

const char *ChallengeSortNode::GetChallengerGamertag() {
    int type = mChallengeRecord->GetChallengeRow().mType;
    bool flag = (type >= 0 && type <= 2);
    if (!flag) {
        flag = (type >= 3 && type <= 5);
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
    int songID = mChallengeRecord->GetChallengeRow().mSongID;
    int timestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(songID);
    if (timestamp > (int)mChallengeRecord->GetChallengeRow().mTimeStamp
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
        if (ownerChallengeScore <= mChallengeRecord->GetChallengeRow().mScore) {
            if (mChallengeRecord->GetUnk48() != mChallengeRecord->GetUnk4c()) {
                app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
            } else {
                label->SetTextToken(gNullStr);
            }
        } else {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (ownerChallengeTimestamp
                <= (int)mChallengeRecord->GetChallengeRow().mTimeStamp) {
                app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
            } else {
                label->SetTextToken(gNullStr);
            }
        }
    } else if (listlabel->Matches("low_gamertag")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );
        if (ownerChallengeScore > mChallengeRecord->GetChallengeRow().mScore) {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (ownerChallengeTimestamp
                > (int)mChallengeRecord->GetChallengeRow().mTimeStamp) {
                app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
            } else {
                label->SetTextToken(gNullStr);
            }
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("right_gamertag")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 1) {
            app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("left_gamertag")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 0) {
            app_label->SetChallengerName(mChallengeRecord->GetUnk48().Str());
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("score")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );

        if (ownerChallengeScore <= mChallengeRecord->GetChallengeRow().mScore) {
            if (mChallengeRecord->GetUnk48() != mChallengeRecord->GetUnk4c()) {
                app_label->SetChallengeScoreLabel(
                    mChallengeRecord->GetChallengeRow().mScore
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (ownerChallengeTimestamp
                <= (int)mChallengeRecord->GetChallengeRow().mTimeStamp) {
                app_label->SetChallengeScoreLabel(
                    mChallengeRecord->GetChallengeRow().mScore
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        }
    } else if (listlabel->Matches("low_score")) {
        int ownerChallengeScore = TheChallengeSortMgr->GetOwnerChallengeScore(
            mChallengeRecord->GetChallengeRow().mSongID
        );

        if (ownerChallengeScore > mChallengeRecord->GetChallengeRow().mScore) {
            int ownerChallengeTimestamp = TheChallengeSortMgr->GetOwnerChallengeTimeStamp(
                mChallengeRecord->GetChallengeRow().mSongID
            );
            if (ownerChallengeTimestamp
                > (int)mChallengeRecord->GetChallengeRow().mTimeStamp) {
                app_label->SetChallengeScoreLabel(
                    mChallengeRecord->GetChallengeRow().mScore
                );
            } else {
                label->SetTextToken(gNullStr);
            }
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("right_score")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 1) {
            app_label->SetChallengeScoreLabel(mChallengeRecord->GetChallengeRow().mScore);
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("left_score")) {
        if (mChallengeRecord->GetUnk48() == mChallengeRecord->GetUnk4c()
            && GetPlayerSide() == 0) {
            app_label->SetChallengeScoreLabel(mChallengeRecord->GetChallengeRow().mScore);
        } else {
            label->SetTextToken(gNullStr);
        }
    } else if (listlabel->Matches("medal")) {
        SetMedalIcon(label);
    } else if (listlabel->Matches("new")) {
        SetNewIcon(label);
    } else if (listlabel->Matches("buy")) {
        SetBuyIcon(label);
    } else if (listlabel->Matches("header_collapse")) {
        label->SetTextToken(gNullStr);
    } else {
        label->SetTextToken(gNullStr);
    }
}

Symbol ChallengeSortNode::Select() {
    static Symbol locked_content_screen("locked_content_screen");
    static Symbol store_loading_screen("store_loading_screen");
    static Symbol show_offers_need_to_sign_in_screen("show_offers_need_to_sign_in_screen");
    static Symbol server_not_available_screen("server_not_available_screen");
    Symbol screen = show_offers_need_to_sign_in_screen;
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);

    if (activeProfile) {
        activeProfile->UpdateOnlineID();
        if (activeProfile->IsSignedIn()
            && ThePlatformMgr.IsSignedIntoLive(activeProfile->GetPadNum())) {
            if (TheRockCentral.IsOnline()) {
                screen = store_loading_screen;
            } else {
                screen = server_not_available_screen;
            }
        }
    }
    static Symbol should_back_to_challenges("should_back_to_challenges");
    switch (mChallengeRecord->GetUnk50()) {
    case 1:
        MILO_ASSERT(false, 0x1a9);
    default: {
        Symbol token = GetToken();
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile && profile->IsContentNew(token)) {
            profile->MarkContentNotNew(token);
        }
        if (UseQuickplayPerformer()) {
            MetaPerformer *pPerformer = MetaPerformer::Current();
            pPerformer->SetSong(GetToken());
        }
        return gNullStr;
    }
    case 2:
    case 3:
        if (screen == store_loading_screen) {
            static Symbol redirect_to_code_redemption("redirect_to_code_redemption");
            UIScreen *storeLoadScreen =
                ObjectDir::Main()->Find<UIScreen>("store_loading_screen");
            storeLoadScreen->SetProperty(redirect_to_code_redemption, 1);
            storeLoadScreen->SetProperty(should_back_to_challenges, 1);
        }
        return screen;
    case 4:
        if (screen == store_loading_screen) {
            static Symbol advertised_songid("advertised_songid");
            UIScreen *storeLoadScreen =
                ObjectDir::Main()->Find<UIScreen>("store_loading_screen");
            storeLoadScreen->SetProperty(advertised_songid, GetSongID());
            storeLoadScreen->SetProperty(should_back_to_challenges, 1);
        }
        return screen;
    }
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
        if (TheHamSongMgr.IsContentUsedForSong(Symbol(contentName), songID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message msg(song_data_mounted, gNullStr);
            msg[0] = GetToken();
            TheUI->Export(msg, false);
        }
    }
}

void ChallengeSortNode::Custom(UIListCustom *list, Hmx::Object *obj) const {
    if (list->Matches("stars")) {
        HamStarsDisplay *starsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(starsDisplay, 0x294);
        starsDisplay->SetShowing(true);
        int type = mChallengeRecord->GetChallengeRow().mType;
        bool check = (type >= 0 && type <= 2);
        if (!check) {
            check = (type >= 3 && type <= 5);
            if (!check) {
                int diff = mChallengeRecord->GetChallengeRow().mDiff;
                starsDisplay->SetSongChallenge((Difficulty)diff);
            }
        }
    }
}

char const *ChallengeSortNode::GetAlbumArtPath() {
    return TheHamSongMgr.GetAlbumArtPath(GetToken());
}

BEGIN_HANDLERS(ChallengeSortNode)
    HANDLE_SUPERCLASS(NavListItemNode)
END_HANDLERS

#pragma endregion
