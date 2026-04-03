#include "net_ham/PlaylistJobs.h"
#include "PlaylistJobs.h"
#include "meta_ham/Playlist.h"
#include "net/JsonUtils.h"
#include "net/json-c/json_object.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_vector.h"
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

void GetPlaylistsJob::GetPlaylists(std::vector<CustomPlaylist> *playlists) {
    if (mResult != 1 || !mJsonResponse) {
        return;
    }
    ::GetPlaylists(mJsonReader, mJsonResponse, playlists);
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

void GetPlaylistJob::GetPlaylist(CustomPlaylist *customP) {
    if (mResult != 1 || !mJsonResponse) {
        return;
    }
    ::GetPlaylist(mJsonReader, mJsonResponse, customP);
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

void AddPlaylistJob::GetPlaylistID(CustomPlaylist *customP) {
    if (mResult == 1 && mJsonResponse) {
        JsonObject *obj = mJsonReader.GetByName(mJsonResponse, "id");
        if (obj) {
            MILO_LOG("===== Added playlistID %i\n", obj->Int());
            customP->SetOnlineID(obj->Int());
        }
    }
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

void GetPlaylist(
    JsonConverter &converter, const JsonObject *obj, CustomPlaylist *cPlaylist
) {
    int size = ((JsonArray *)obj)->GetSize();
    for (unsigned int i = 0; i < size; i++) {
        JsonObject *val = converter.GetValue(((JsonArray *)obj), i);
        JsonObject *val2 = converter.GetValue(((JsonArray *)val), 0);
        cPlaylist->AddSong(val2->Int());
    }
}

void GetPlaylists(
    JsonConverter &converter,
    const JsonObject *obj,
    stlpmtx_std::vector<CustomPlaylist> *playlists
) {
    int size = ((JsonArray *)obj)->GetSize();
    for (unsigned int i = 0; i < size; i++) {
        JsonObject *val = converter.GetValue(((JsonArray *)obj), i);
        {
            playlists->push_back(CustomPlaylist());
        }
        CustomPlaylist &p = playlists->back();
        p.SetOnlineID(converter.GetValue(((JsonArray *)val), 0)->Int());
        CustomPlaylist &p2 = playlists->back();
        p2.SetName(converter.GetValue(((JsonArray *)val), 1)->Str());
    }
}
