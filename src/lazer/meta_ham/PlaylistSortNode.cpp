#include "PlaylistSortNode.h"
#include "meta_ham/PlaylistSortNode.h"

#include "Accomplishment.h"
#include "AppLabel.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "HamStarsDisplay.h"
#include "MetaPerformer.h"
#include "PlaylistSortMgr.h"
#include "ProfileMgr.h"
#include "ui/UI.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "utl/Symbol.h"

#pragma region PlaylistSortNode

Symbol PlaylistSortNode::OnSelect() {
    if (UseQuickplayPerformer()) {
        MetaPerformer::Current()->ResetSongs();
    }
    Select();
    if (unk44 == gNullStr) {
        auto obj = ObjectDir::Main()->Find<UIScreen>(unk44.Str(), true);
        TheUI->PushScreen(obj);
        return 0;
    }
    else {
        return ThePlaylistSortMgr->unk78.front().GetName();
    }
}

Symbol PlaylistSortNode::Select() {
    static Symbol locked_content_screen("locked_content_screen");
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
    if (TheProfileMgr.IsContentUnlocked(unk44)) {
        if (activeProfile) {
            if (activeProfile->IsContentNew(unk44)) {
                activeProfile->MarkContentNotNew(unk44);
            }
        }
        if (UseQuickplayPerformer()) {
            MetaPerformer *cur = MetaPerformer::Current();
            cur->SetSong(unk48->GetName());
        }
        return 0;
    }
    else {
        //TheLockedContentPanel->SetUp
        return locked_content_screen;
    }
    return 0;
}

void PlaylistSortNode::Custom(UIListCustom *list, Hmx::Object *obj) const {
    if (list->Matches("stars")) {
        HamStarsDisplay *pStarsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarsDisplay, 0x143);
        pStarsDisplay->SetShowing(true);
        int id = TheHamSongMgr.GetSongIDFromShortName(unk44);
        pStarsDisplay->SetSong(id);
    }
}

void PlaylistSortNode::Text(UIListLabel *pHamLabel, UILabel *label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x122);
    if (pHamLabel->Matches("label")) {
        MILO_ASSERT(pHamLabel, 0x126);
        pHamLabel->SetNote("test");
    }
    else {
        if (pHamLabel->Matches("duration")) {
            MILO_ASSERT(pHamLabel, 0x12d);
            int duration = unk48->GetDuration();
            if (duration > 0) {
                label->SetTimeHMS(duration, false);
                return;
            }

        }
    }
}

#pragma endregion PlaylistSortNode
#pragma region PlaylistHeaderNode

BEGIN_HANDLERS(PlaylistHeaderNode)
HANDLE_EXPR(get_challenge_count, mChallengeCount)
HANDLE_SUPERCLASS(NavListSortNode)
END_HANDLERS

PlaylistHeaderNode::PlaylistHeaderNode(NavListItemSortCmp *cmp, Symbol s, bool b)
    : NavListHeaderNode(cmp, s, b), mChallengeCount() {}

void PlaylistHeaderNode::OnHighlight() {
    SongPreview *preview = ThePlaylistSortMgr->GetSongPreview();
    preview->Start(GetToken(), 0);
}

Symbol PlaylistHeaderNode::OnSelect() {
    bool mode;
    if (!ThePlaylistSortMgr->IsInHeaderMode()) {
        ThePlaylistSortMgr->SetHeaderMode(true);
        mode = 1;
    }
    else {
        mode = 0;
    }
    ThePlaylistSortMgr->SetEnteringHeaderMode(mode);
    return gNullStr;
}

Symbol PlaylistHeaderNode::OnSelectDone() {
    if (ThePlaylistSortMgr->IsInHeaderMode() && !ThePlaylistSortMgr->EnteringHeaderMode()) {
        ThePlaylistSortMgr->SetHeaderMode(false);
    }
    ThePlaylistSortMgr->unk78.back(); // function call to 0x7c??? WHERE IS IT COMING FROM?
    ThePlaylistSortMgr->GetCurrentSort()->BuildItemList();
    return gNullStr;
}

bool PlaylistHeaderNode::IsActive() const {
    return ThePlaylistSortMgr->HeadersSelectable() ? IsActive() : false;
}

NavListSortNode *PlaylistHeaderNode::GetFirstActive() {
    FOREACH(it, Children()) {
        if ((*it)->GetFirstActive()) return *it;
    }
}

char const *PlaylistHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");
    auto curSort = ThePlaylistSortMgr->GetCurrentSort()->GetSortName();
    if (curSort == by_album && curSort != singles && !Children().empty()) {
        return Children().front()->GetAlbumArtPath();
    }
    return 0;
}

#pragma endregion PlaylistHeaderNode
