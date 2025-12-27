#include "net_ham/LeaderboardJobs.h"
#include "hamobj/Difficulty.h"
#include "net/JsonUtils.h"
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
    Hmx::Object *callback, const HamProfile *, int songID
)
    : RCJob("leaderboards/getminileaderboard/", callback) {
    static Symbol song_id("song_id");
    mSongID = songID;
    DataPoint dataP;
    dataP.AddPair(song_id, songID);
    SetDataPoint(dataP);
}

void GetRows(JsonConverter &c, const JsonObject *o, std::vector<LeaderboardRow> *rows) {
    JsonArray *a = const_cast<JsonArray *>(static_cast<const JsonArray *>(o));
    unsigned int aSize = a->GetSize();
    for (int i = 0; i < aSize; i++) {
        JsonArray *cur = static_cast<JsonArray *>(c.GetValue(a, i));
        LeaderboardRow row;
        row.unk0 = c.GetValue(cur, 1)->Str();
        row.unk8 = c.GetValue(cur, 0)->Int();
        row.unkc = c.GetValue(cur, 4)->Int();
        row.unk10 = c.GetValue(cur, 3)->Int();
        row.unk14 = c.GetValue(cur, 7)->Int();
        row.unk18 = (Difficulty)c.GetValue(cur, 2)->Int();
        row.unk1c = false;
        row.unk1d = c.GetValue(cur, 5)->Bool();
        row.unk1e = c.GetValue(cur, 6)->Bool();
        const char *str = c.GetValue(cur, 8)->Str();
        if (strcmp(str, gNullStr) != 0) {
            row.unk20 = 0;
            while (*str != '\0') {
                row.unk20 = row.unk20 * 10 + (*str++ - 0x30);
            }
        }
        rows->push_back(row);
    }
}

void GetLeaderboardByPlayerJob::GetRows(std::vector<LeaderboardRow> *rows) {
    if (mResult == 1 && mJsonResponse) {
        ::GetRows(mJsonReader, mJsonResponse, rows);
    }
}

void GetMiniLeaderboardJob::GetRows(std::vector<LeaderboardRow> *rows) {
    if (mResult == 1 && mJsonResponse) {
        ::GetRows(mJsonReader, mJsonResponse, rows);
    }
}
