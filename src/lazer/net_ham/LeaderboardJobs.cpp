#include "net_ham/LeaderboardJobs.h"
#include "net_ham/RCJobDingo.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

GetLeaderboardByPlayerJob::GetLeaderboardByPlayerJob(
    Hmx::Object *callback,
    HamProfile *,
    int songID,
    int typeID,
    int modeID,
    int numRows,
    unsigned int ui
)
    : RCJob("leaderboards/getleaderboard/", callback) {
    static Symbol song_id("song_id");
    static Symbol type_id("type_id");
    static Symbol mode_id("mode_id");
    static Symbol num_rows("num_rows");
    unkb0 = ui;
    DataPoint dataP;
    dataP.AddPair(song_id, songID);
    dataP.AddPair(type_id, typeID);
    dataP.AddPair(mode_id, modeID);
    dataP.AddPair(num_rows, numRows);
    SetDataPoint(dataP);
}

GetMiniLeaderboardJob::GetMiniLeaderboardJob(
    Hmx::Object *callback, HamProfile const *, int songID
)
    : RCJob("leaderboards/getminileaderboard/", callback) {
    static Symbol song_id("song_id");
    unkb0 = songID;
    DataPoint dataP;
    dataP.AddPair(song_id, songID);
    SetDataPoint(dataP);
}
