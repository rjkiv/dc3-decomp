#pragma once
#include "meta_ham/Playlist.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "stl/_vector.h"

class GetPlaylistsJob : public RCJob {
public:
    GetPlaylistsJob(Hmx::Object *, char const *);
    void GetPlaylists(std::vector<CustomPlaylist> *);
};

class GetPlaylistJob : public RCJob {
public:
    GetPlaylistJob(Hmx::Object *, char const *, int);
    void GetPlaylist(CustomPlaylist *);
};

class AddPlaylistJob : public RCJob {
public:
    AddPlaylistJob(Hmx::Object *, char const *, Playlist *);
    AddPlaylistJob(Hmx::Object *, char const *, char const *, char const *);
    void GetPlaylistID(CustomPlaylist *);
};

class DeletePlaylistJob : public RCJob {
public:
    DeletePlaylistJob(Hmx::Object *, char const *, int);
};

class EditPlaylistJob : public RCJob {
public:
    EditPlaylistJob(Hmx::Object *, char const *, Playlist *);
};

class SyncAvailableDynamicPlaylistsJob : public RCJob {
public:
    SyncAvailableDynamicPlaylistsJob(Hmx::Object *, char const *, int);
};
