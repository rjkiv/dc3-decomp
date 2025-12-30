#pragma once
#include "hamobj/HamNavProvider.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/PlaylistSongProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

class SongSelectPlaylistProvider : public HamNavProvider {
public:
    virtual int NumData() const;
    virtual Symbol DataSymbol(int) const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    SongSelectPlaylistProvider();
    Playlist *GetPlaylist(int) const;

    std::vector<Playlist *> m_vPlaylists; // 0x40
    CustomPlaylist unk4c;
};

class SongSelectPlaylistPanel : public HamPanel {
public:
    // Hmx::Object
    virtual ~SongSelectPlaylistPanel();
    OBJ_CLASSNAME(SongSelectPlaylistPanel)
    OBJ_SET_TYPE(SongSelectPlaylistPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Unload();
    virtual void FinishLoad();

    NEW_OBJ(SongSelectPlaylistPanel)

    SongSelectPlaylistPanel();
    int GetSelectedPlaylistIndex();
    void UpdateSongs(int);
    Playlist *GetSelectedPlaylist();
    void SelectPlaylist();
    void DeletePlaylist();
    void Refresh();
    bool IsSelectingCustomPlaylist();

    SongSelectPlaylistProvider *m_pSongSelectPlaylistProvider; // 0x3c
    PlaylistSongProvider *m_pPlaylistSongProvider; // 0x40
};
