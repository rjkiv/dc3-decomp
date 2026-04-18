#include "SongSortNode.h"

#include "HamProfile.h"
#include "NavListSortMgr.h"
#include "SongSortMgr.h"
#include "meta/SongMgr.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamStarsDisplay.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/LockedContentPanel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/NavListSort.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "ui/UIListCustom.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <cstdio>

int SongHeaderNode::GetItemCount() { return mDiscSongs + mDLCSongs; }

SongHeaderNode::SongHeaderNode(NavListItemSortCmp *cmp, Symbol s, bool b)
    : NavListHeaderNode(cmp, s, b), mDiscSongs(0), mDLCSongs(0) {}

SongHeaderNode::~SongHeaderNode() { delete mDateTime; }

void SongHeaderNode::OnHighlight() {
    auto song = TheSongSortMgr->GetSongPreview();
    song->Start(0, 0);
    SetCollapseStateIcon(true);
}

void SongHeaderNode::SetCollapseStateIcon(bool b) const {
    Symbol s = gNullStr;
    UILabel *iconLabel = GetCollapseIconLabel();
    if (iconLabel) {
        static Symbol header_open_icon("header_open_icon");
        static Symbol header_open_highlighted_icon("header_open_highlighted_icon");
        static Symbol header_closed_icon("header_closed_icon");
        static Symbol header_closed_highlighted_icon("header_closed_highlighted_icon");
        if (TheSongSortMgr->IsInHeaderMode()) {
            if (b) {
                s = header_closed_highlighted_icon;
            } else {
                s = header_closed_icon;
            }
        } else {
            if (b) {
                s = header_open_highlighted_icon;
            } else {
                s = header_open_icon;
            }
        }
        iconLabel->SetTextToken(s);
    }
}

Symbol SongHeaderNode::OnSelect() {
    if (!TheSongSortMgr->IsInHeaderMode()) {
        TheSongSortMgr->SetHeaderMode(true);
        TheSongSortMgr->SetEnteringHeaderMode(true);
    } else {
        TheSongSortMgr->SetEnteringHeaderMode(false);
        TheSongSortMgr->SetExitingHeaderMode(true);
    }
    return gNullStr;
}

Symbol SongHeaderNode::Select() {
    return SelectChildren(mChildren, mDiscSongs + mDLCSongs);
}

Symbol SongHeaderNode::OnSelectDone() {
    if (TheSongSortMgr->EnteringHeaderMode()) {
        TheSongSortMgr->SetEnteringHeaderMode(false);
    }
    if (TheSongSortMgr->ExitingHeaderMode()) {
        if (TheSongSortMgr->IsInHeaderMode()) {
            TheSongSortMgr->SetHeaderMode(false);
        }
        TheSongSortMgr->SetExitingHeaderMode(false);
    }
    TheSongSortMgr->OnEnter();
    auto curSort = TheSongSortMgr->GetCurrentSort();
    curSort->BuildItemList();
    return gNullStr;
}

void SongHeaderNode::SetItemCountString(UILabel *lbl) const {
    char buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // why
    sprintf(buf, "(%d)", mDiscSongs + mDLCSongs);
    Symbol s = buf;
    lbl->SetPrelocalizedString(String(s));
}

bool SongHeaderNode::IsActive() const {
    if (!TheSongSortMgr->HeadersSelectable())
        return false;
    else
        return IsActive();
}

void SongHeaderNode::UpdateItemCount(NavListItemNode *itemnode) {
    SongSortNode *songsn = dynamic_cast<SongSortNode *>(itemnode);
    if (songsn != nullptr) {
        if (songsn->Record()->Metadata()->IsDownload()) {
            mDLCSongs++;
        } else {
            mDiscSongs++;
        }
    }
}

void SongHeaderNode::Renumber(std::vector<NavListSortNode *> &vec) {
    SetStartIndex(vec.size());
    if (TheSongSortMgr->GetHeadersSelectable()) {
        vec.push_back(this);
        TheSongSortMgr->AddHeaderIndex(StartIndex());
    }
    if (!TheSongSortMgr->IsInHeaderMode()) {
        FOREACH (it, Children()) {
            (*it)->Renumber(vec);
        }
    }
}

char const *SongHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");

    if (TheSongSortMgr->GetCurrentSort()->GetSortName() == by_album
        && GetToken() != singles) {
        auto node = mChildren.begin();
        if (node != mChildren.end())
            return (*node)->GetAlbumArtPath();
    }
    return 0;
}

Symbol SongSortNode::Select() {
    static Symbol invalid_version_screen("invalid_version_screen");
    if (!unk_0x48->Metadata()->IsVersionOK()) {
        return invalid_version_screen;
    } else {
        static Symbol locked_content_screen("locked_content_screen");
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        Symbol token = GetToken();
        if (!TheProfileMgr.IsContentUnlocked(token)) {
            TheLockedContentPanel->SetUp(token);
            return locked_content_screen;
        }
        if (profile != nullptr) {
            if (profile->IsContentNew(token)) {
                profile->MarkContentNotNew(token);
            }
        }
        if (UseQuickplayPerformer()) {
            auto *perf = MetaPerformer::Current();
            perf->SetSong(GetToken());
        }
        return gNullStr;
    }
}

Symbol SongSortNode::OnSelect() {
    if (UseQuickplayPerformer() && !TheSongSortMgr->GetSetlistMode()) {
        MetaPerformer::Current()->ResetSongs();
    }
    Symbol sel = Select();
    if (sel != gNullStr) {
        auto obj = ObjectDir::Main()->Find<UIScreen>(sel.Str(), true);
        TheUI->PushScreen(obj);
        return gNullStr;
    } else if (!TheSongSortMgr->GetSetlistMode()) {
        return TheSongSortMgr->MoveOn();
    } else {
        TheSongSortMgr->OnSetlistChanged();
        return gNullStr;
    }
}

void SongSortNode::OnContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0x1a0);
    if (!TheContentMgr.RefreshInProgress()) {
        int songID = TheHamSongMgr.GetSongIDFromShortName(GetToken());
        if (TheHamSongMgr.IsContentUsedForSong(contentName, songID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message msg(song_data_mounted, gNullStr);
            msg[0] = GetToken();
            TheUI->Export(msg, false);
        }
    }
}

void SongSortNode::SetInPlaylist(bool b) { unk_0x4C = b; }

void SongHeaderNode::Text(UIListLabel *list_label, UILabel *ui_label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(ui_label);
    MILO_ASSERT(app_label, 0xb6);

    if (unk44) {
        if (list_label->Matches("famousby")) {
            static Symbol store_famous_by("store_famous_by");
            ui_label->SetTextToken(store_famous_by);
            return;
        }
        if (list_label->Matches("famousby_group")) {
            app_label->SetFromSongSelectNode(this);
            return;
        }
    } else {
        if (list_label->Matches("group")) {
            app_label->SetFromSongSelectNode(this);
            return;
        }
    }

    if (list_label->Matches("sort_header")) {
        app_label->SetFromSongSelectNode(this);
        return;
    } else if (list_label->Matches("song_count")) {
        SetItemCountString(ui_label);
        return;
    } else if (list_label->Matches("header_collapse")) {
        TheSongSortMgr->GetHighlightItem() == this ? SetCollapseStateIcon(true)
                                                   : SetCollapseStateIcon(false);
        return;
    } else {
        ui_label->SetTextToken(gNullStr);
    }
}

NavListSortNode *SongHeaderNode::GetFirstActive() {
    FOREACH (it, mChildren) {
        auto firstActive = (*it)->GetFirstActive();
        if (firstActive) {
            return TheSongSortMgr->HeadersSelectable() ? this : firstActive;
        }
    }
    return nullptr;
}

bool SongSortNode::IsCoverSong(Symbol shortname) const {
    if (!TheSongMgr.HasSong(shortname, false)) {
        return false;
    } else {
        const HamSongMetadata *songData =
            TheHamSongMgr.Data(TheSongMgr.GetSongIDFromShortName(shortname));
        MILO_ASSERT(songData, 458);
        return songData->IsCover();
    }
}

bool SongSortNode::IsMedley() const {
    auto shortname = unk_0x48->ShortName();
    if (!TheSongMgr.HasSong(shortname, false)) {
        return false;
    } else {
        const HamSongMetadata *songData =
            TheHamSongMgr.Data(TheSongMgr.GetSongIDFromShortName(shortname));
        MILO_ASSERT(songData, 474);
        return songData->IsMedley();
    }
}

bool SongSortNode::IsFake() const {
    auto shortname = unk_0x48->ShortName();
    if (!TheSongMgr.HasSong(shortname, false)) {
        return false;
    } else {
        const HamSongMetadata *songData =
            TheHamSongMgr.Data(TheSongMgr.GetSongIDFromShortName(shortname));
        MILO_ASSERT(songData, 490);
        return songData->IsFake();
    }
}

void SongSortNode::Text(UIListLabel *ull, UILabel *ul) const {
    static Symbol score("score");
    static Symbol disc("disc");
    static Symbol dlc("dlc");
    const Symbol shortname = unk_0x48->ShortName();
    if (ull->Matches("song")) {
        AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
        MILO_ASSERT(app_label, 513);
        static Symbol by_artist("by_artist");
        if (TheSongSortMgr->GetCurrentSortName() != by_artist) {
            app_label->SetBlacklightSongName(shortname, -1, false);
        } else
            app_label->SetBlacklightSongName(shortname, -1, false);
    } else if (ull->Matches("lock")) {
        bool locked = !TheProfileMgr.IsContentUnlocked(shortname);
        AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
        MILO_ASSERT(app_label, 533);
        app_label->SetLocked(locked);
    } else if (ull->Matches("new")) {
        bool nu = false;
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            nu = profile->IsContentNew(shortname);
        }
        AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
        MILO_ASSERT(app_label, 546);
        app_label->SetNew(nu);
    } else if (ull->Matches("download")) {
        const HamSongMetadata *pData = unk_0x48->Metadata();
        MILO_ASSERT(pData, 552);
        if (TheHamUI.IsBlacklightMode()) {
            ul->SetTextToken(gNullStr);
            return;
        }
        static Symbol ham1("ham1");
        static Symbol ham2("ham2");
        static Symbol ham3("ham3");
        if (pData->GameOrigin() == ham1) {
            ul->SetIcon('d');
        } else if (pData->GameOrigin() == ham2) {
            ul->SetIcon('j');
        } else if (pData->GameOrigin() != ham3) {
            AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
            MILO_ASSERT(app_label, 580);
            app_label->SetDownload(true);
        } else {
            ul->SetTextToken(gNullStr);
        }
    } else if (ull->Matches("header_text")) {
        if (!IsHeader()) {
            ul->SetTextToken(gNullStr);
        } else {
            ul->SetTextToken(HeaderText());
        }
    } else if (ull->Matches("asmadefamous")) {
        static Symbol by_artist("by_artist");

        if (TheSongSortMgr->GetCurrentSortName() != by_artist
            || !IsCoverSong(shortname)) {
            ul->SetTextToken(gNullStr);
        } else {
            ul->SetTextToken(ull->GetDefaultText());
        }
    } else if (ull->Matches("artist")) {
        Symbol sName = unk_0x48->ShortName();
        if (TheAccomplishmentMgr->IsUnlockableAsset(sName)) {
            ul->SetTextToken(gNullStr);
            return;
        }
        AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
        MILO_ASSERT(app_label, 646);
        static Symbol by_artist("by_artist");
        if (TheSongSortMgr->GetCurrentSortName() != by_artist) {
            app_label->SetArtistName(shortname, true);
            return;
        }
        app_label->SetBlacklightSongName(shortname, -1, false);

    } else if (ull->Matches("in_playlist")) {
        if (unk_0x4C) {
            ul->SetIcon('i');
        } else {
            ul->SetTextToken(gNullStr);
        }
    } else if (ull->Matches("header_collapse")) {
        ul->SetTextToken(gNullStr);
    } else if (!ull->Matches("song_prefix")) {
        ul->SetTextToken(gNullStr);
    } else if (!IsHeader()) {
        if (TheHamUI.IsBlacklightMode()) {
            static Symbol song_select_song_prefix("song_select_song_prefix");
            ul->SetTextToken(song_select_song_prefix);
        } else {
            ul->SetTextToken(gNullStr);
        }
    } else {
        ul->SetTextToken(gNullStr);
    }
}

void SongSortNode::Custom(UIListCustom *ulc, Hmx::Object *obj) const {
    if (ulc->Matches("stars")) {
        HamStarsDisplay *pStarsDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarsDisplay, 700);
        pStarsDisplay->SetShowing(true);
        Symbol shortname = GetToken();
        pStarsDisplay->SetSong(TheHamSongMgr.GetSongIDFromShortName(shortname));
    }
}

const char *SongSortNode::GetArtist(void) const {
    auto *songData = unk_0x48->Metadata();
    MILO_ASSERT(songData, 723);
    return songData->Artist();
}

Symbol SongSortNode::GetToken() const { return unk_0x48->ShortName(); }

BEGIN_HANDLERS(SongHeaderNode)
    HANDLE_EXPR(get_song_count, mDiscSongs + mDLCSongs)
    HANDLE_EXPR(get_download_count, mDLCSongs)
    HANDLE_EXPR(get_disc_count, mDiscSongs)
    HANDLE_SUPERCLASS(NavListHeaderNode)
END_HANDLERS

BEGIN_HANDLERS(SongSortNode)
    HANDLE_MEMBER_PTR(const_cast<HamSongMetadata *>(unk_0x48->Metadata()))
    HANDLE_SUPERCLASS(NavListItemNode)
END_HANDLERS

void SongFunctionNode::OnHighlight() { TheSongSortMgr->GetSongPreview()->Start(0, 0); }

bool SongFunctionNode::IsActive() const {
    static Symbol play_setlist("play_setlist");
    return unk4c != play_setlist;
}

Symbol SongFunctionNode::OnSelect() {
    static Symbol playlists("playlists");
    static Symbol finish_setlist("finish_setlist");
    static Symbol play_setlist("play_setlist");
    static Symbol random_song("random_song");
    static Symbol career("career");
    if (GetToken() == playlists) {
        static Symbol move_on_quickplay_playlist("move_on_quickplay_playlist");
        UIPanel *pPanel = ObjectDir::Main()->Find<UIPanel>("song_select_panel");
        MILO_ASSERT(pPanel, 0x12d);
        static Message msg(move_on_quickplay_playlist);
        pPanel->HandleType(msg);
    } else if (GetToken() == random_song) {
        Symbol song = TheHamSongMgr.GetRandomSong();
        static Symbol special_select_song("special_select_song");
        UIPanel *pPanel = ObjectDir::Main()->Find<UIPanel>("song_select_panel");
        MILO_ASSERT(pPanel, 0x139);
        static Message msg(special_select_song, gNullStr);
        msg[0] = song;
        pPanel->HandleType(msg);
    }
    return gNullStr;
}

void SongFunctionNode::Text(UIListLabel *listlabel, UILabel *label) const {
    if (listlabel->Matches("function")) {
        AppLabel *app_label = dynamic_cast<AppLabel *>(label);
        MILO_ASSERT(app_label, 0x150);
        app_label->SetFromSongSelectNode(this);
    } else {
        label->SetTextToken(gNullStr);
    }
}
