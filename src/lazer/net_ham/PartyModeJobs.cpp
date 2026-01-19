#include "net_ham/PartyModeJobs.h"
#include "PartyModeJobs.h"
#include "RCJobDingo.h"
#include "game/PartyModeMgr.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/Playlist.h"
#include "net/JsonUtils.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

SetPartyOptionsJob::SetPartyOptionsJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("partymode/setpartyoptions/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol diff("diff");
    static Symbol shortened("shortened");
    static Symbol perform_it("perform_it");
    static Symbol dance_battle("dance_battle");
    static Symbol bust_a_move("bust_a_move");
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol holla_back("holla_back");
    static Symbol strike_a_pose("strike_a_pose");
    static Symbol playlist_type("playlist_type");
    static Symbol playlist_id("playlist_id");
    static Symbol perform("perform");
    static Symbol bustamove("bustamove");

    dataP.AddPair(pid, onlineID);
    dataP.AddPair(diff, ThePartyModeMgr->GetDifficulty());
    dataP.AddPair(shortened, ThePartyModeMgr->UseFullLengthSongs() == 0);
    dataP.AddPair(perform_it, ThePartyModeMgr->IsModeIncluded(perform));
    dataP.AddPair(dance_battle, ThePartyModeMgr->IsModeIncluded(dance_battle));
    dataP.AddPair(bust_a_move, ThePartyModeMgr->IsModeIncluded(bustamove));
    dataP.AddPair(rhythm_battle, ThePartyModeMgr->IsModeIncluded(rhythm_battle));
    dataP.AddPair(holla_back, ThePartyModeMgr->IsModeIncluded(holla_back));
    dataP.AddPair(strike_a_pose, ThePartyModeMgr->IsModeIncluded(strike_a_pose));

    Playlist *playlist = ThePartyModeMgr->GetPlaylist();
    int playlistType = 0;
    int playlistId = 0;
    if (playlist) {
        if (playlist->IsCustom()) {
            playlistId = playlist->GetOnlineID();
            if (playlistId == -1) {
                playlistId = 0;
            }
        } else {
            if (playlist->GetUnk8()) {
                static Symbol playlists("playlists");
                playlistType = 1;
                DataArray *sysConfig = SystemConfig(playlists);
                for (int i = 1; i < sysConfig->Size(); i++) {
                    DataArray *playlistEntry = sysConfig->Array(i);
                    MILO_ASSERT(playlistEntry, 0x51);
                    Symbol entrySym = playlistEntry->Sym(0);
                    if (playlist->GetName() == entrySym) {
                        playlistId = i;
                        break;
                    }
                }
            } else {
                playlistType = 2;
                playlistId = GetDynamicPlaylistID(playlist->GetName());
            }
        }
    }

    dataP.AddPair(playlist_type, playlistType);
    dataP.AddPair(playlist_id, playlistId);
    SetDataPoint(dataP);
}

GetPartyOptionsJob::GetPartyOptionsJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("partymode/getpartyoptions/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol locale("locale");
    Symbol sysLang = SystemLanguage();
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(locale, sysLang.Str());
    SetDataPoint(dataP);
}

void GetPartyOptionsJob::GetOptions() {
    if (mResult == 1) {
        JsonObject *response = mJsonResponse;
        JsonConverter &reader = mJsonReader;
        if (response) {
            JsonObject *getByName = reader.GetByName(response, "diff");
            if (getByName) {
                ThePartyModeMgr->SetDifficulty((Difficulty)getByName->Int());
            }
            getByName = mJsonReader.GetByName(response, "shortened");
            if (getByName) {
                ThePartyModeMgr->SetUseFullLengthSongs(getByName->Bool() == 0);
            }

            static Symbol perform("perform");
            static Symbol dance_battle("dance_battle");
            static Symbol bustamove("bustamove");
            static Symbol rhythm_battle("rhythm_battle");
            static Symbol holla_back("holla_back");
            static Symbol strike_a_pose("strike_a_pose");

            getByName = mJsonReader.GetByName(response, "perform_it");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(perform, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "dance_battle");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(dance_battle, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "bust_a_move");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(bustamove, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "rhythm_battle");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(rhythm_battle, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "holla_back");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(holla_back, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "strike_a_pose");
            if (getByName) {
                ThePartyModeMgr->ToggleIncludedModeOn(strike_a_pose, getByName->Bool());
            }
            getByName = mJsonReader.GetByName(response, "playlist_id");
            if (getByName && getByName->Int() != 0) {
                getByName = mJsonReader.GetByName(response, "playlist_name");
                if (getByName) {
                    Playlist *playlist =
                        TheHamSongMgr.GetPlaylistWithLocalizedName(getByName->Str());
                    if (playlist) {
                        ThePartyModeMgr->SetPlaylist(playlist);
                    }
                }
            }
        }
    }
}

GetPartySongQueueJob::GetPartySongQueueJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("partymode/getpartysongqueue/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

AddSongToPartySongQueueJob::AddSongToPartySongQueueJob(
    Hmx::Object *callback, const char *onlineID, int songID
)
    : RCJob("partymode/addsongtopartysongqueue/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(song_id, songID);
    SetDataPoint(dataP);
}

SyncPlayerSongsJob::SyncPlayerSongsJob(
    Hmx::Object *callback, const char *onlineID, String &songIDs
)
    : RCJob("partymode/syncplayersongs/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_ids("song_ids");
    MILO_ASSERT(onlineID, 0x125);
    MILO_ASSERT(songIDs != gNullStr, 0x126);
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(song_ids, songIDs.c_str());
    SetDataPoint(dataP);
}

DeleteSongFromPartySongQueueJob::DeleteSongFromPartySongQueueJob(
    Hmx::Object *callback, const char *onlineID, int songID
)
    : RCJob("partymode/deletesongfrompartysongqueue/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol id("id");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(id, songID);
    SetDataPoint(dataP);
}

void GetSongQueue(JsonConverter &c, const JsonObject *o, std::list<SongQueueRow> *rows) {
    JsonArray *a = const_cast<JsonArray *>(static_cast<const JsonArray *>(o));
    unsigned int aSize = a->GetSize();
    for (int i = 0; i < aSize; i++) {
        JsonArray *cur = static_cast<JsonArray *>(c.GetValue(a, i));
        SongQueueRow row;
        row.unk0 = c.GetValue(cur, 0)->Int();
        row.mSongID = c.GetValue(cur, 1)->Int();
        row.unk8 = c.GetValue(cur, 2)->Str();
        row.unk10 = c.GetValue(cur, 3)->Str();
        rows->push_back(row);
    }
}

void GetPartySongQueueJob::GetSongQueue(std::list<SongQueueRow> *rows) {
    if (mResult == 1 && mJsonResponse) {
        ::GetSongQueue(mJsonReader, mJsonResponse, rows);
    }
}
