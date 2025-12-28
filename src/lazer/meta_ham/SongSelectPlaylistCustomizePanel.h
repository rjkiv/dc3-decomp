#pragma once
#include "meta_ham/HamPanel.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/PlaylistSongProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

class SongSelectPlaylistCustomizePanel : public HamPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(SongSelectPlaylistCustomizePanel)
    OBJ_SET_TYPE(SongSelectPlaylistCustomizePanel)
    virtual DataNode Handle(DataArray *, bool);

    // HamPanel
    virtual void Unload();
    virtual void FinishLoad();

    NEW_OBJ(SongSelectPlaylistCustomizePanel)

    SongSelectPlaylistCustomizePanel();
    void UpdatePlaylistName(HamLabel *);
    bool IsPlaylistEmpty() const;
    bool IsPlaylistFull() const;
    void UpdateSongs();
    void SelectSong(Symbol);
    void CancelSong();
    void CancelSongAtIndex(int);
    void Refresh();

    PlaylistSongProvider *m_pPlaylistSongProvider; // 0x3c
    Playlist *m_pPlaylist; // 0x40

private:
    void InsertSong(Symbol, int);
    void MoveSong(Symbol, int);
};
