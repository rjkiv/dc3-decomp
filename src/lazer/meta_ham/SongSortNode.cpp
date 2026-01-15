#include "SongSortNode.h"

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

void SongHeaderNode::SetCollapseStateIcon(bool) const {
    auto *label = GetCollapseIconLabel();
    static Symbol header_open_icon("header_open_icon");
    static Symbol header_open_highlighted_icon("header_open_highlighted_icon");
    static Symbol header_closed_icon("header_closed_icon");
    static Symbol header_closed_highlighted_icon("header_closed_highlighted_icon");
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
    if (TheSongSortMgr->HeadersSelectable()) { // this is adding in a bunch of other insts
                                               // for some reason
        vec.push_back(this);
        TheSongSortMgr->AddHeaderIndex(StartIndex());
    }
    if (!TheSongSortMgr->IsInHeaderMode()) {
        FOREACH (it, Children()) {
            (*it)->Renumber(vec);
        }
    }
}

Symbol SongSortNode::Select() {
    static Symbol invalid_version_screen("invalid_version_screen");
    if (!unk_0x48->Metadata()->IsVersionOK()) {
        return invalid_version_screen;
    } else {
        static Symbol locked_content_screen("locked_content_screen");
        auto *profile = TheProfileMgr.GetActiveProfile(true);
        Symbol token = GetToken();
        if (!TheProfileMgr.IsContentUnlocked(token)) {
            TheLockedContentPanel->SetUp(token);
            return invalid_version_screen; // ???
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
    if (UseQuickplayPerformer() && TheSongSortMgr) {
        MetaPerformer::Current()->ResetSongs();
    }
    Symbol sel = Select();
    if (sel != gNullStr) {
        auto obj = ObjectDir::Main()->Find<UIScreen>(sel.Str(), true);
        TheUI->PushScreen(obj);
        return gNullStr;
    } else {
        return TheSongSortMgr->MoveOn();
    }
}

void SongSortNode::OnContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0x1a0);
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

void SongSortNode::SetInPlaylist(bool b) { unk_0x4C = b; }

void SongHeaderNode::Text(UIListLabel *, UILabel *ui_label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(ui_label);
    MILO_ASSERT(app_label, 182);
    // if (unk44) {
    //     static Symbol store_famous_by("store_famous_by");
    // }
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
        // TheSongSortMgr->
        if (!IsCoverSong(shortname)) {
            ul->SetTextToken(gNullStr);
        } else {
            ul->SetTextToken(ull->GetDefaultText());
        }
    } else if (ull->Matches("artist")) {
        if (!TheAccomplishmentMgr->IsUnlockableAsset(unk_0x48->ShortName())) {
            ul->SetTextToken(gNullStr);
        }
        AppLabel *app_label = dynamic_cast<AppLabel *>(ul);
        MILO_ASSERT(app_label, 646);
        static Symbol by_artist("by_artist");
        app_label->SetArtistName(shortname, true);
    } else if (ull->Matches("in_playlist")) {
        if (!unk_0x4C) {
            ul->SetTextToken(gNullStr);
        } else {
            ul->SetIcon('i');
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
    Symbol mode = GetToken();
    if (mode == playlists) {
        static Symbol move_on_quickplay_playlist("move_on_quickplay_playlist");
        auto pPanel = ObjectDir::Main()->Find<UIPanel>("song_select_panel", true);
        MILO_ASSERT(pPanel, 0x12d);
        static Message msg(move_on_quickplay_playlist);
        HandleType(msg);
    } else {
        if (mode != random_song) {
            return gNullStr;
        }
        auto song = TheHamSongMgr.GetRandomSong();
        static Symbol special_select_song("special_select_song");
        auto pPanel = ObjectDir::Main()->Find<UIPanel>("song_select_panel", true);
        MILO_ASSERT(pPanel, 0x139);
        static Message msg(special_select_song, gNullStr);
        msg->Node(2).Sym() = song;
        HandleType(msg);
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
