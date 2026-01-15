#include "PlaylistSortNode.h"
#include "hamobj/HamLabel.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/LockedContentPanel.h"
#include "meta_ham/PlaylistSortNode.h"

#include "Accomplishment.h"
#include "AppLabel.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "HamStarsDisplay.h"
#include "MetaPerformer.h"
#include "PlaylistSortMgr.h"
#include "ProfileMgr.h"
#include "obj/Msg.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "ui/UI.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

#pragma region PlaylistSortNode

Symbol PlaylistSortNode::OnSelect() {
    if (UseQuickplayPerformer()) {
        MetaPerformer::Current()->ResetSongs();
    }
    Symbol sel = Select();
    if (sel != gNullStr) {
        auto obj = ObjectDir::Main()->Find<UIScreen>(sel.Str(), true);
        TheUI->PushScreen(obj);
        return gNullStr;
    } else {
        return ThePlaylistSortMgr->MoveOn();
    }
}

Symbol PlaylistSortNode::Select() {
    static Symbol locked_content_screen("locked_content_screen");
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
    Symbol token = GetToken();

    if (!TheProfileMgr.IsContentUnlocked(token)) {
        TheLockedContentPanel->SetUp(token);
        return locked_content_screen;

    } else {
        if (activeProfile) {
            if (activeProfile->IsContentNew(token)) {
                activeProfile->MarkContentNotNew(token);
            }
        }
        if (UseQuickplayPerformer()) {
            MetaPerformer *pPerformer = MetaPerformer::Current();
            pPerformer->SetSong(GetToken());
        }
        return gNullStr;
    }
}

void PlaylistSortNode::Custom(UIListCustom *list, Hmx::Object *obj) const {
    if (list->Matches("stars")) {
        HamStarsDisplay *pStarsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarsDisplay, 0x143);
        pStarsDisplay->SetShowing(true);
        Symbol token = GetToken();
        int id = TheHamSongMgr.GetSongIDFromShortName(token);
        pStarsDisplay->SetSong(id);
    }
}

void PlaylistSortNode::Text(UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x122);
    if (uiListLabel->Matches("label")) {
        HamLabel *pHamLabel = app_label;
        MILO_ASSERT(pHamLabel, 0x126);
        pHamLabel->SetTextToken(GetToken());
    } else if (uiListLabel->Matches("duration")) {
        HamLabel *pHamLabel = app_label;
        MILO_ASSERT(pHamLabel, 0x12d);
        int duration = unk48->GetDuration();
        if (duration > 0) {
            uiLabel->SetTimeHMS(duration, false);
            return;
        }
        pHamLabel->SetTextToken(gNullStr);
    } else {
        uiLabel->SetTextToken(gNullStr);
    }
}

void PlaylistSortNode::OnContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0xfe);
    if (!TheContentMgr.RefreshInProgress()) {
        int songID = TheHamSongMgr.GetSongIDFromShortName(GetToken());
        if (TheHamSongMgr.IsContentUsedForSong(contentName, songID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message song_data_mounted_message(song_data_mounted, gNullStr);
            song_data_mounted_message[0] = GetToken();
            TheUI->Export(song_data_mounted_message, false);
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
    preview->Start(0, 0);
}

Symbol PlaylistHeaderNode::OnSelect() {
    bool mode;
    if (!ThePlaylistSortMgr->IsInHeaderMode()) {
        ThePlaylistSortMgr->SetHeaderMode(true);
        mode = 1;
    } else {
        mode = 0;
    }
    ThePlaylistSortMgr->SetEnteringHeaderMode(mode);
    return gNullStr;
}

Symbol PlaylistHeaderNode::OnSelectDone() {
    if (ThePlaylistSortMgr->IsInHeaderMode()
        && !ThePlaylistSortMgr->EnteringHeaderMode()) {
        ThePlaylistSortMgr->SetHeaderMode(false);
    }
    ThePlaylistSortMgr->OnEnter();
    ThePlaylistSortMgr->GetCurrentSort()->BuildItemList();
    return gNullStr;
}

bool PlaylistHeaderNode::IsActive() const {
    return ThePlaylistSortMgr->HeadersSelectable() ? IsActive() : false;
}

NavListSortNode *PlaylistHeaderNode::GetFirstActive() {
    FOREACH (it, Children()) {
        if ((*it)->GetFirstActive())
            return *it;
    }
}

char const *PlaylistHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");
    auto curSort = ThePlaylistSortMgr->GetCurrentSort()->GetSortName();
    if (curSort == by_album && GetToken() != singles && HasChildren()) {
        return FirstChild()->GetAlbumArtPath();
    }
    return 0;
}

void PlaylistHeaderNode::Renumber(stlpmtx_std::vector<NavListSortNode *> &vec) {
    mStartIx = vec.size();
    if (ThePlaylistSortMgr->HeadersSelectable()) {
        vec.push_back(this);
        ThePlaylistSortMgr->AddHeaderIndex(mStartIx);
    }
    if (!ThePlaylistSortMgr->IsInHeaderMode()) {
        FOREACH (it, mChildren) {
            (*it)->Renumber(vec);
        }
    }
}

void PlaylistHeaderNode::Text(UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x89);
    if (uiListLabel->Matches("header_label")) {
        HamLabel *pHamLabel = app_label; // why
        MILO_ASSERT(pHamLabel, 0x8e);
        pHamLabel->SetTextToken(GetToken());
    } else {
        app_label->SetTextToken(uiListLabel->GetDefaultText());
    }
}

#pragma endregion PlaylistHeaderNode
