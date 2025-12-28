#include "meta_ham/SongSelectPlaylistCustomizePanel.h"
#include "SongSelectPlaylistCustomizePanel.h"
#include "hamobj/HamLabel.h"
#include "macros.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/PlaylistSongProvider.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"

SongSelectPlaylistCustomizePanel::SongSelectPlaylistCustomizePanel()
    : m_pPlaylistSongProvider(), m_pPlaylist() {}

void SongSelectPlaylistCustomizePanel::Unload() {
    UIPanel::Unload();
    if (m_pPlaylistSongProvider)
        delete m_pPlaylistSongProvider;
    m_pPlaylistSongProvider = 0;
    m_pPlaylist = 0;
}

void SongSelectPlaylistCustomizePanel::FinishLoad() {
    UIPanel::FinishLoad();
    MILO_ASSERT(!m_pPlaylistSongProvider, 0x54);
    m_pPlaylistSongProvider = new PlaylistSongProvider();
    m_pPlaylistSongProvider->SetName("custom_playlist_provider", ObjectDir::Main());
    MetaPerformer *pPerformer = MetaPerformer::Current();
    MILO_ASSERT(pPerformer, 0x59);
    m_pPlaylist = pPerformer->GetPlaylist();
}

void SongSelectPlaylistCustomizePanel::UpdatePlaylistName(HamLabel *i_pLabel) {
    MILO_ASSERT(i_pLabel, 0x88);
    AppLabel *pAppLabel = dynamic_cast<AppLabel *>(i_pLabel);
    MILO_ASSERT(pAppLabel, 0x8b);
    MILO_ASSERT(m_pPlaylist, 0x8d);
    pAppLabel->SetPlaylistName(m_pPlaylist, m_pPlaylist->IsEmpty() == false, false);
}

bool SongSelectPlaylistCustomizePanel::IsPlaylistEmpty() const {
    MILO_ASSERT(m_pPlaylist, 0x95);
    return m_pPlaylist->IsEmpty();
}

bool SongSelectPlaylistCustomizePanel::IsPlaylistFull() const {
    MILO_ASSERT(m_pPlaylist, 0x9d);
    return m_pPlaylist->IsFull();
}

void SongSelectPlaylistCustomizePanel::UpdateSongs() {
    MILO_ASSERT(m_pPlaylistSongProvider, 0x7b);
    MILO_ASSERT(m_pPlaylist, 0x7c);
    m_pPlaylistSongProvider->UpdateList(m_pPlaylist, true);
    static Message update_song_list("update_song_list");
    Handle(update_song_list, true);
}

void SongSelectPlaylistCustomizePanel::InsertSong(Symbol s, int i) {
    MILO_ASSERT(m_pPlaylist, 0xb8);
    MILO_ASSERT(!m_pPlaylist->IsFull(), 0xb9);
    m_pPlaylist->InsertSong(TheHamSongMgr.GetSongIDFromShortName(s, true), i);
    UpdateSongs();
}

void SongSelectPlaylistCustomizePanel::MoveSong(Symbol s, int i) {
    MILO_ASSERT(m_pPlaylist, 0xc4);
    if (0 <= i) {
        m_pPlaylist->MoveSong(m_pPlaylistSongProvider->DataIndex(s), i);
        UpdateSongs();
    }
}

void SongSelectPlaylistCustomizePanel::SelectSong(Symbol s) {
    MILO_ASSERT(m_pPlaylist, 0x2a);
    MILO_ASSERT(!m_pPlaylist->IsFull(), 0x2b);
    m_pPlaylist->AddSong(TheHamSongMgr.GetSongIDFromShortName(s));
    UpdateSongs();
}

void SongSelectPlaylistCustomizePanel::CancelSong() {
    MILO_ASSERT(m_pPlaylist, 0x36);
    MILO_ASSERT(!m_pPlaylist->IsEmpty(), 0x37);
    m_pPlaylist->RemoveSong();
    UpdateSongs();
}

void SongSelectPlaylistCustomizePanel::CancelSongAtIndex(int index) {
    MILO_ASSERT(m_pPlaylist, 0x41);
    MILO_ASSERT(!m_pPlaylist->IsEmpty(), 0x42);
    m_pPlaylist->RemoveSongAtIndex(index);
    UpdateSongs();
}

void SongSelectPlaylistCustomizePanel::Refresh() {
    MetaPerformer *pPerformer = MetaPerformer::Current();
    MILO_ASSERT(pPerformer, 0x6b);
    MILO_ASSERT(m_pPlaylist, 0x6e);
    UpdateSongs();
    MILO_ASSERT(m_pPlaylistSongProvider, 0x72);
    static Message update_playlist_song_provider("update_playlist_song_provider");
    if (m_pPlaylistSongProvider) {
        // something
    }
    Handle(update_playlist_song_provider, true);
}

BEGIN_HANDLERS(SongSelectPlaylistCustomizePanel)
    HANDLE_ACTION(select_song, SelectSong(_msg->Sym(2)))
    HANDLE_ACTION(cancel_song, CancelSong())
    HANDLE_ACTION(cancel_song_at_index, CancelSongAtIndex(_msg->Int(2)))
    HANDLE_ACTION(update_songs, UpdateSongs())
    HANDLE_ACTION(update_playlist_name, UpdatePlaylistName(_msg->Obj<HamLabel>(2)))
    HANDLE_EXPR(is_playlist_empty, IsPlaylistEmpty())
    HANDLE_EXPR(is_playlist_full, IsPlaylistFull())
    HANDLE_EXPR(
        is_valid_song, TheHamSongMgr.GetSongIDFromShortName(_msg->Sym(2), false) != false
    )
    HANDLE_ACTION(s, Refresh())
    HANDLE_ACTION(get_playlist_provider, 0)
    HANDLE_ACTION(swap_songs, m_pPlaylist->SwapSongs(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(insert_song, InsertSong(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(move_song, MoveSong(_msg->Sym(2), _msg->Int(3)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
