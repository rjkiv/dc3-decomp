#include "net_ham/PartyModeJobs.h"
#include "PartyModeJobs.h"
#include "RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

SongQueueRow::SongQueueRow() {}

SongQueueRow::SongQueueRow(SongQueueRow const &sqr)
    : unk0(sqr.unk0), unk4(sqr.unk4), unk8(sqr.unk8), unk10(sqr.unk10) {}

GetPartyOptionsJob::GetPartyOptionsJob(Hmx::Object *o, char const *onlineID)
    : RCJob("partymode/getpartyoptions/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol locale("locale");
    Symbol sysLang = SystemLanguage();
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(locale, DataNode(sysLang.Str()));
    SetDataPoint(dataP);
}

GetPartySongQueueJob::GetPartySongQueueJob(Hmx::Object *o, char const *onlineID)
    : RCJob("partymode/getpartysongqueue/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, DataNode(onlineID));
    SetDataPoint(dataP);
}

AddSongToPartySongQueueJob::AddSongToPartySongQueueJob(
    Hmx::Object *o, char const *onlineID, int songID
)
    : RCJob("partymode/addsongtopartysongqueue/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(song_id, DataNode(songID));
    SetDataPoint(dataP);
}

SyncPlayerSongsJob::SyncPlayerSongsJob(
    Hmx::Object *o, char const *onlineID, String &songIDs
)
    : RCJob("partymode/syncplayersongs/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_ids("song_ids");
    MILO_ASSERT(onlineID, 0x125);
    MILO_ASSERT(songIDs != gNullStr, 0x126);
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(song_ids, DataNode(songIDs.c_str()));
    SetDataPoint(dataP);
}

DeleteSongFromPartySongQueueJob::DeleteSongFromPartySongQueueJob(
    Hmx::Object *o, char const *onlineID, int songID
)
    : RCJob("partymode/deletesongfrompartysongqueue/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol id("id");
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(id, DataNode(songID));
    SetDataPoint(dataP);
}
