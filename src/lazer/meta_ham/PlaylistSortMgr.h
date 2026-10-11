#pragma once
#include "NavListSortMgr.h"
#include "game/PartyModeMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/FitnessGoalMgr.h"
#include "meta_ham/Playlist.h"
#include "net_ham/PlaylistJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

struct CmdAddPlaylistToRC : public QueueableCommand {
    virtual int GetType() { return 4; }
    CmdAddPlaylistToRC(Playlist *pl) : playlist(pl) {}
    Playlist *playlist;
};

struct CmdDeletePlaylistFromRC : public QueueableCommand {
    virtual int GetType() { return 6; }
    CmdDeletePlaylistFromRC(int id) : playlistID(id) {}
    int playlistID;
};

struct CmdEditPlaylist : public QueueableCommand {
    virtual int GetType() { return 5; }
    CmdEditPlaylist(Playlist *pl) : playlist(pl) {}
    Playlist *playlist;
};

struct CmdGetPlaylistFromRC : public QueueableCommand {
    virtual int GetType() { return 3; }
    CmdGetPlaylistFromRC(int id) : playlistID(id) {}
    int playlistID;
};

struct CmdGetPlaylistsFromRC : public QueueableCommand {
    virtual int GetType() { return 1; }
    CmdGetPlaylistsFromRC() {}
};

struct CmdResolvePlaylists : public QueueableCommand {
    virtual int GetType() { return 2; }
    CmdResolvePlaylists() {}
};

class PlaylistSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SelectionIs(Symbol) { return 0; }
    virtual Symbol MoveOn() { return 0; }
    virtual void OnEnter();

    int ConvertListIndexToPlaylistIndex(int);
    Playlist *GetPlaylist(int);
    void OnDeletePlaylistFromRC(Playlist *);
    void UpdateList();

    static void Init(SongPreview &);

    const std::vector<Playlist *> &GetPlaylists() const { return mPlaylists; }
    void AddPlaylist(Playlist *p) { mPlaylists.push_back(p); }

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
    void QueueCmdGetPlaylistsFromRC(int);
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
    DataNode OnMsg(SmartGlassMsg const &);
    DataNode OnMsg(RCJobCompleteMsg const &);

    std::vector<Playlist *> mPlaylists; // 0x78
    CustomPlaylist m_cBlankPlaylist; // 0x84
    String mCurrProfileName; // 0xb0
    String mCurrProfileOnlineIDs; // 0xb8
    std::list<QueueableCommand *> mCommands; // 0xc0
    bool mIsProccessingCmd; // 0xc8
    RCJob *mCurrJob; // 0xcc
    std::vector<CustomPlaylist> mPlaylistsFromRC; // 0xd0
};

extern PlaylistSortMgr *ThePlaylistSortMgr;
