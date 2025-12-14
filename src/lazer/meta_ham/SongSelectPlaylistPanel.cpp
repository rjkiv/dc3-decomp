#include "meta_ham/SongSelectPlaylistPanel.h"
#include "HamPanel.h"
#include "SongSelectPlaylistPanel.h"
#include "macros.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

#pragma region SongSelectPlaylistPanel

SongSelectPlaylistPanel::SongSelectPlaylistPanel()
    : m_pSongSelectPlaylistProvider(0), m_pPlaylistSongProvider(0) {}

SongSelectPlaylistPanel::~SongSelectPlaylistPanel() {}

void SongSelectPlaylistPanel::Unload() { UIPanel::Unload(); }

void SongSelectPlaylistPanel::FinishLoad() { UIPanel::FinishLoad(); }

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

void SongSelectPlaylistPanel::UpdateSongs(int) {
    MILO_ASSERT(m_pPlaylistSongProvider, 0xf7);
    MILO_ASSERT(m_pSongSelectPlaylistProvider, 0xf9);
    // need PlaylistSortMgr
    static Message update_songcount("update_songcount");
    // something
    static Message update_song_list("update_song_list");
    Handle(update_song_list, true);
}

BEGIN_HANDLERS(SongSelectPlaylistPanel)
    HANDLE_ACTION(select_playlist, SelectPlaylist())
    HANDLE_ACTION(delete_playlist, DeletePlaylist())
    HANDLE_ACTION(is_selecting_custom_playlist, GetSelectedPlaylist()) // fix
    HANDLE_EXPR(
        is_waiting_for_active_profile, TheProfileMgr.HasActiveProfileWithInvalidSaveData()
    )
    HANDLE_ACTION(update_songs, UpdateSongs(_msg->Int(2)))
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion SongSelectPlaylistPanel
#pragma region SongSelectPlaylistProvider

SongSelectPlaylistProvider::SongSelectPlaylistProvider() : unk4c() {
    static Symbol never_use("never_use");
    unk4c.mName = never_use;
}

int SongSelectPlaylistProvider::NumData() const { return m_vPlaylists.size(); }

Symbol SongSelectPlaylistProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x61);
    return gNullStr;
}

void SongSelectPlaylistProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *
) const {
    MILO_ASSERT(i_iData < NumData(), 0x34);
    Playlist *pPlaylist = GetPlaylist(i_iData);
    MILO_ASSERT(pPlaylist, 0x37);
    if (uiListLabel->Matches("label")) {
        // need AppLabel
    }
}

Playlist *SongSelectPlaylistProvider::GetPlaylist(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vPlaylists.size(), 0x54);
    return m_vPlaylists[i_iIndex];
}

#pragma endregion SongSelectPlaylistProvider
