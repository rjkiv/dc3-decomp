#pragma once
#include "NavListSortMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/Playlist.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

class PlaylistSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual void OnEnter();
    int ConvertListIndexToPlaylistIndex(int);
    Playlist *GetPlaylist(int);
    void OnDeletePlaylistFromRC(Playlist *);
    void UpdateList();

    static void Init(SongPreview &);

    std::vector<Playlist> unk78;
    CustomPlaylist unk84;
    String unkb0;
    String unkb8;
    std::list<Playlist> unkc0;
    std::vector<CustomPlaylist> unkd0;

private:
    virtual ~PlaylistSortMgr();

    PlaylistSortMgr(SongPreview &);
    bool IsProfileChanged();
    void OnSmartGlassListen(int);
    void SendPassiveMsg(Symbol);
    void StartCmdGetPlaylistsFromRC();
    void FakeAddPlaylistsToRC();
    void BroadcastSyncMsg(Symbol);
    void ResolvePlaylists();
    void StartCmdGetPlaylistFromRC();
    void StartCmdAddPlaylistToRC();
    void StartCmdDeletePlaylistFromRC();
    void StartCmdEditPlaylist();
    void HandleCmdChangeProfileOnlineID();
    void HandleCmdResolvePlaylists();
    void ProcessNextCommand();
    void QueueCmdChangeProfileOnlineID(String);
    void QueueCmdGetPlaylistsFromRC();
    void QueueCmdResolvePlaylists();
    void QueueCmdGetPlaylistFromRC(int);
    void HandleCmdGetPlaylistFromRC();
    void QueueCmdAddPlaylistToRC(Playlist *);
    void HandleCmdAddPlaylistToRC();
    void QueueCmdDeletePlaylistFromRC(int);
    void HandleCmdDeletePlaylistFromRC();
    void QueueCmdEditPlaylist(Playlist *);
    void HandleCmdEditPlaylist();
    bool HasValidProfile();
    void UpdateCurrPlaylistWithRC();
    void HandleCmdGetPlaylistsFromRC();
    // DataNode OnMsg(SmartGlassMsg const &);
    DataNode OnMsg(RCJobCompleteMsg const &);
};

extern PlaylistSortMgr *ThePlaylistSortMgr;
