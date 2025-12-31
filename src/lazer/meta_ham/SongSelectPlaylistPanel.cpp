#include "meta_ham/SongSelectPlaylistPanel.h"
#include "HamPanel.h"
#include "SongSelectPlaylistPanel.h"
#include "macros.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/PlaylistSongProvider.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SaveLoadManager.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

#pragma region SongSelectPlaylistProvider

SongSelectPlaylistProvider::SongSelectPlaylistProvider() : unk4c() {
    static Symbol never_use("never_use");
    unk4c.SetName(never_use);
}

int SongSelectPlaylistProvider::NumData() const { return m_vPlaylists.size(); }

Symbol SongSelectPlaylistProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x61);
    return gNullStr;
}

void SongSelectPlaylistProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x34);
    Playlist *pPlaylist = GetPlaylist(i_iData);
    MILO_ASSERT(pPlaylist, 0x37);
    if (uiListLabel->Matches("label")) {
        AppLabel *pHamLabel = dynamic_cast<AppLabel *>(uiLabel);
        MILO_ASSERT(pHamLabel, 0x3d);
        if (!pPlaylist->IsCustom() || !pPlaylist->IsEmpty()) {
            pHamLabel->SetPlaylistName(pPlaylist, true, true);
        } else {
            static Symbol playlist_create("playlist_create");
            uiLabel->SetTextToken(playlist_create);
        }
    } else {
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
    }
}

Playlist *SongSelectPlaylistProvider::GetPlaylist(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vPlaylists.size(), 0x54);
    return m_vPlaylists[i_iIndex];
}

#pragma endregion SongSelectPlaylistProvider
#pragma region SongSelectPlaylistPanel

SongSelectPlaylistPanel::SongSelectPlaylistPanel()
    : m_pSongSelectPlaylistProvider(0), m_pPlaylistSongProvider(0) {}

SongSelectPlaylistPanel::~SongSelectPlaylistPanel() {}

void SongSelectPlaylistPanel::Unload() {
    UIPanel::Unload();
    RELEASE(m_pSongSelectPlaylistProvider);
    RELEASE(m_pPlaylistSongProvider);
}

void SongSelectPlaylistPanel::FinishLoad() {
    UIPanel::FinishLoad();
    MILO_ASSERT(!m_pSongSelectPlaylistProvider, 0xd1);
    m_pSongSelectPlaylistProvider = new SongSelectPlaylistProvider();
    MILO_ASSERT(!m_pPlaylistSongProvider, 0xd4);
    m_pPlaylistSongProvider = new PlaylistSongProvider();
}

int SongSelectPlaylistPanel::GetSelectedPlaylistIndex() {
    if (mState != kUp) {
        return 0;
    } else {
        static Message get_selected_playlist_index("get_selected_playlist_index");
        DataNode node = Handle(get_selected_playlist_index, true);
        return node.Int();
    }
}

void SongSelectPlaylistPanel::SelectPlaylist() {
    Playlist *pPlaylist = GetSelectedPlaylist();
    MILO_ASSERT(pPlaylist, 0xa4);
    static Symbol never_use("never_use");
    MILO_ASSERT(pPlaylist->GetName() != never_use, 0xa7);
    MetaPerformer *pPerformer = MetaPerformer::Current();
    MILO_ASSERT(pPerformer, 0xaa);
    pPerformer->SetPlaylist(pPlaylist);
}

void SongSelectPlaylistPanel::UpdateSongs(int i) {
    MILO_ASSERT(m_pPlaylistSongProvider, 0xf7);
    MILO_ASSERT(m_pSongSelectPlaylistProvider, 0xf9);
    Playlist *pPlaylist = ThePlaylistSortMgr->GetPlaylist(i);
    m_pPlaylistSongProvider->UpdateList(pPlaylist, false);
    static Message update_songcount("update_songcount", 0);
    int num = 0;
    if (pPlaylist) {
        num = pPlaylist->GetNumSongs();
    } else {
        num = 0;
    }
    update_songcount[0] = num;
    Handle(update_songcount, true);
    static Message update_song_list("update_song_list");
    Handle(update_song_list, true);
}

void SongSelectPlaylistPanel::DeletePlaylist() {
    Playlist *pPlaylist = GetSelectedPlaylist();
    MILO_ASSERT(pPlaylist, 0xb3);
    static Symbol never_use("never_use");
    MILO_ASSERT(pPlaylist->GetName() != never_use, 0xb6);
    for (int i = pPlaylist->GetNumSongs(); i != 0;) {
        i--; // cant put it in the for loop constructor huh...
        pPlaylist->RemoveSong();
    }
    if (TheSaveLoadMgr)
        TheSaveLoadMgr->AutoSave();
    ThePlaylistSortMgr->OnDeletePlaylistFromRC(pPlaylist);
}

Playlist *SongSelectPlaylistPanel::GetSelectedPlaylist() {
    MILO_ASSERT(m_pSongSelectPlaylistProvider, 0x78);
    return ThePlaylistSortMgr->GetPlaylist(GetSelectedPlaylistIndex());
}

bool SongSelectPlaylistPanel::IsSelectingCustomPlaylist() {
    Playlist *pPlaylist = GetSelectedPlaylist();
    return !pPlaylist ? false : pPlaylist->IsCustom();
}

void SongSelectPlaylistPanel::Refresh() {
    MILO_ASSERT(m_pSongSelectPlaylistProvider, 0xe4);
    ThePlaylistSortMgr->UpdateList();
    static Message update_playlist_provider("update_playlist_provider", 0);
    update_playlist_provider[0] = ThePlaylistSortMgr;
    Handle(update_playlist_provider, true);
    UpdateSongs(GetSelectedPlaylistIndex());
    MILO_ASSERT(m_pPlaylistSongProvider, 0xee);
    static Message update_song_provider("update_song_provider", 0);
    update_song_provider[0] = m_pPlaylistSongProvider;
    Handle(update_song_provider, true);
}

BEGIN_HANDLERS(SongSelectPlaylistPanel)
    HANDLE_ACTION(select_playlist, SelectPlaylist())
    HANDLE_ACTION(delete_playlist, DeletePlaylist())
    HANDLE_EXPR(is_selecting_custom_playlist, IsSelectingCustomPlaylist())
    HANDLE_EXPR(
        is_waiting_for_active_profile, TheProfileMgr.HasActiveProfileWithInvalidSaveData()
    )
    HANDLE_ACTION(update_songs, UpdateSongs(_msg->Int(2)))
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion SongSelectPlaylistPanel
