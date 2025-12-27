#include "net_ham/PartyModeJobs.h"
#include "PartyModeJobs.h"
#include "RCJobDingo.h"
#include "net/JsonUtils.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

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
        row.unk4 = c.GetValue(cur, 1)->Int();
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
