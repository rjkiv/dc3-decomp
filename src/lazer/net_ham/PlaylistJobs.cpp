#include "net_ham/PlaylistJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/DataPointMgr.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

GetPlaylistsJob::GetPlaylistsJob(Hmx::Object *callback, char const *onlineID)
    : RCJob("playlists/getplaylists/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

GetPlaylistJob::GetPlaylistJob(Hmx::Object *callback, char const *onlineID, int playlistID)
    : RCJob("playlists/getplaylist/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol id("id");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(id, playlistID);
    SetDataPoint(dataP);
}

AddPlaylistJob::AddPlaylistJob(
    Hmx::Object *callback, char const *onlineID, Playlist *playlist
)
    : RCJob("playlists/addplaylist/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol title("title");
    static Symbol song_ids("song_ids");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(title, playlist->GetName().Str());
    String songIDs = "";
    songIDs += MakeString("%i", playlist->GetSong(0));
    for (int i = 1; i < playlist->GetNumSongs(); i++) {
        songIDs += MakeString(",%i", playlist->GetSong(i));
    }
    dataP.AddPair(song_ids, songIDs.c_str());
    SetDataPoint(dataP);
}

AddPlaylistJob::AddPlaylistJob(
    Hmx::Object *callback,
    char const *onlineID,
    char const *playlistTitle,
    char const *songIDs
)
    : RCJob("playlists/addplaylist/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol title("title");
    static Symbol song_ids("song_ids");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(title, playlistTitle);
    dataP.AddPair(song_ids, songIDs);
    SetDataPoint(dataP);
}

DeletePlaylistJob::DeletePlaylistJob(
    Hmx::Object *callback, char const *onlineID, int playlistID
)
    : RCJob("playlists/deleteplaylist/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol id("id");
    MILO_LOG("===== deleting playlistID %i for user %s\n", playlistID, onlineID);
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(id, playlistID);
    SetDataPoint(dataP);
}

EditPlaylistJob::EditPlaylistJob(
    Hmx::Object *callback, char const *onlineID, Playlist *playlist
)
    : RCJob("playlists/editplaylist/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol id("id");
    static Symbol song_ids("song_ids");
    dataP.AddPair(pid, onlineID);
    int ID = playlist->GetOnlineID();
    dataP.AddPair(id, ID);
    String songIDs = "";
    songIDs += MakeString("%i", playlist->GetSong(0));
    for (int i = 1; i < playlist->GetNumSongs(); i++) {
        songIDs += MakeString(",%i", playlist->GetSong(i));
    }
    dataP.AddPair(song_ids, songIDs.c_str());
    SetDataPoint(dataP);
}

SyncAvailableDynamicPlaylistsJob::SyncAvailableDynamicPlaylistsJob(
    Hmx::Object *callback, char const *onlineID, int flagAmount
)
    : RCJob("playlists/syncavailabledynamicplaylists/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol flags("flags");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(flags, flagAmount);
    SetDataPoint(dataP);
}
