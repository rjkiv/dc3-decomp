#include "net_ham/ChallengeSystemJobs.h"
#include "meta_ham/HamProfile.h"
#include "net/JsonUtils.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "utl/DataPointMgr.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"

FlauntScoreJob::FlauntScoreJob(Hmx::Object *callback, FlauntScoreData &data)
    : RCJob("leaderboards/flauntscore/", callback) {
    DataPoint pt;
    static Symbol song_id("song_id");
    static Symbol pid("pid");
    static Symbol score("score");
    static Symbol diff("diff");
    static Symbol xp("xp");
    pt.AddPair(song_id, data.mStatus->mSongID);
    pt.AddPair(pid, data.mProfile ? data.mProfile->GetOnlineID()->ToString() : "N/A");
    pt.AddPair(score, data.mStatus->mScore);
    pt.AddPair(diff, data.mStatus->mDiff);
    pt.AddPair(xp, data.mProfile->GetMetagameRank()->RankNumber());
    SetDataPoint(pt);
}

GetPlayerChallengesJob::GetPlayerChallengesJob(
    Hmx::Object *callback, std::vector<HamProfile *> &profiles
)
    : RCJob("leaderboards/getplayerchallenges/", callback) {
    DataPoint pt;
    static Symbol pid000("pid000");
    static Symbol pid001("pid001");
    if (profiles.size() > 1) {
        pt.AddPair(pid001, profiles[1]->GetOnlineID()->ToString());
    }
    pt.AddPair(pid000, profiles[0]->GetOnlineID()->ToString());
    SetDataPoint(pt);
}

GetOfficialChallengesJob::GetOfficialChallengesJob(Hmx::Object *callback)
    : RCJob("leaderboards/getchallenges/", callback) {
    DataPoint pt;
    SetDataPoint(pt);
}

void GetOfficialChallengesJob::GetRows(
    std::vector<ChallengeRow> &rows, double &dref, bool &bref
) {
    if (mResult == 1) {
        JsonConverter &reader = mJsonReader;
        JsonObject *response = mJsonResponse;
        if (response) {
            static Symbol challenge_gold("challenge_gold");
            static Symbol challenge_silver("challenge_silver");
            static Symbol challenge_bronze("challenge_bronze");
            std::vector<ChallengeRow> calcedRows;
            DateTime startTime;
            JsonObject *startName = reader.GetByName(response, "start_time");
            if (startName) {
                startTime.ParseDate(startName->Str());
                String dateStr;
                startTime.ToDateString(dateStr);
                MILO_LOG(
                    ">>>>>>>>>> official_challenge_start_time = %s\n", dateStr.c_str()
                );
            }
            JsonObject *nextStartName = reader.GetByName(response, "next_start_time");
            if (nextStartName) {
                if (strcmp(nextStartName->Str(), "0000-00-00") == 0) {
                    dref = -1;
                } else {
                    DateTime nextStartTime;
                    nextStartTime.ParseDate(nextStartName->Str());
                    String dateStr;
                    nextStartTime.ToDateString(dateStr);
                    MILO_LOG(">>>>>>>>>> next_start_time = %s\n", dateStr.c_str());
                    DateTime systemDt;
                    GetSystemDateAndTime(systemDt);
                    dateStr.erase();
                    systemDt.ToString(dateStr);
                    MILO_LOG(">>>>>>>>>> system_time = %s\n", dateStr.c_str());
                    dref = nextStartTime.DiffSeconds(systemDt);
                    if (dref < 0) {
                        dref = 0;
                    }
                    MILO_LOG(">>>>>>>>>> expireTime = %f\n", (float)dref);
                }
            }
            JsonObject *songIDName = reader.GetByName(response, "hmx_song_id");
            if (songIDName) {
                int songID = songIDName->Int();
                if (songID != 0) {
                    ChallengeRow localRows[3];
                    JsonObject *goldName = reader.GetByName(response, "hmx_gold_score");
                    if (goldName) {
                        localRows[0].mScore = goldName->Int();
                        localRows[0].mType = ChallengeRow::kChallengeHmxGold;
                        localRows[0].mGamertag =
                            Localize(challenge_gold, nullptr, TheLocale);
                    }
                    JsonObject *silverName =
                        reader.GetByName(response, "hmx_silver_score");
                    if (silverName) {
                        localRows[1].mScore = silverName->Int();
                        localRows[1].mType = ChallengeRow::kChallengeHmxSilver;
                        localRows[1].mGamertag =
                            Localize(challenge_silver, nullptr, TheLocale);
                    }
                    JsonObject *bronzeName =
                        reader.GetByName(response, "hmx_bronze_score");
                    if (bronzeName) {
                        localRows[2].mScore = bronzeName->Int();
                        localRows[2].mType = ChallengeRow::kChallengeHmxBronze;
                        localRows[2].mGamertag =
                            Localize(challenge_bronze, nullptr, TheLocale);
                    }
                    for (int i = 0; i < 3; i++) {
                        localRows[i].mSongID = songID;
                        localRows[i].unk0 = i;
                        JsonObject *artistName = reader.GetByName(response, "hmx_artist");
                        if (artistName) {
                            localRows[i].mArtist = artistName->Str();
                        }
                        JsonObject *songName =
                            reader.GetByName(response, "hmx_song_name");
                        if (songName) {
                            localRows[i].mSongTitle = songName->Str();
                        }
                        JsonObject *diffName = reader.GetByName(response, "hmx_diff");
                        if (diffName) {
                            localRows[i].mDiff = diffName->Int();
                        }
                        localRows[i].unk2c = "";
                        localRows[i].mTimeStamp = startTime.ToCode();
                        localRows[i].mChallengerXp = 0;
                        calcedRows.push_back(localRows[i]);
                    }
                }
            }
            JsonObject *dlcSongIDName = reader.GetByName(response, "dlc_song_id");
            if (dlcSongIDName) {
                int dlcSongID = dlcSongIDName->Int();
                if (dlcSongID != 0) {
                    ChallengeRow localRows[3];
                    JsonObject *goldName = reader.GetByName(response, "dlc_gold_score");
                    if (goldName) {
                        localRows[0].mScore = goldName->Int();
                        localRows[0].mType = ChallengeRow::kChallengeDlcGold;
                        localRows[0].mGamertag =
                            Localize(challenge_gold, nullptr, TheLocale);
                    }
                    JsonObject *silverName =
                        reader.GetByName(response, "dlc_silver_score");
                    if (silverName) {
                        localRows[1].mScore = silverName->Int();
                        localRows[1].mType = ChallengeRow::kChallengeDlcSilver;
                        localRows[1].mGamertag =
                            Localize(challenge_silver, nullptr, TheLocale);
                    }
                    JsonObject *bronzeName =
                        reader.GetByName(response, "dlc_bronze_score");
                    if (bronzeName) {
                        localRows[2].mScore = bronzeName->Int();
                        localRows[2].mType = ChallengeRow::kChallengeDlcBronze;
                        localRows[2].mGamertag =
                            Localize(challenge_bronze, nullptr, TheLocale);
                    }
                    for (int i = 0; i < 3; i++) {
                        localRows[i].mSongID = dlcSongID;
                        localRows[i].unk0 = i;
                        JsonObject *artistName = reader.GetByName(response, "dlc_artist");
                        if (artistName) {
                            localRows[i].mArtist = artistName->Str();
                        }
                        JsonObject *songName =
                            reader.GetByName(response, "dlc_song_name");
                        if (songName) {
                            localRows[i].mSongTitle = songName->Str();
                        }
                        JsonObject *diffName = reader.GetByName(response, "dlc_diff");
                        if (diffName) {
                            localRows[i].mDiff = diffName->Int();
                        }
                        localRows[i].unk2c = "";
                        localRows[i].mTimeStamp = startTime.ToCode();
                        localRows[i].mChallengerXp = 0;
                        calcedRows.push_back(localRows[i]);
                    }
                }
            }
            bref = false;
            if (rows.size() == calcedRows.size()) {
                for (int i = 0; i < calcedRows.size(); i++) {
                    if (rows[i] != calcedRows[i]) {
                        bref = true;
                        break;
                    }
                }
            } else {
                bref = true;
            }
            rows.swap(calcedRows);
        }
    }
}

GetChallengeBadgeCountsJob::GetChallengeBadgeCountsJob(
    Hmx::Object *callback, std::vector<HamProfile *> &profiles
)
    : RCJob("leaderboards/getchallengebadgecounts/", callback) {
    MILO_LOG("***********************************\n");
    MILO_LOG("GetChallengeBadgeCountsJob::GetChallengeBadgeCountsJob()\n");
    MILO_LOG(MakeString("   profiles.size()   = %d\n", (int)profiles.size()));
    for (int i = 0; i < profiles.size(); i++) {
        MILO_LOG(MakeString(
            "   profiles[%d]       = '%s'\n", i, profiles[i]->GetOnlineID()->ToString()
        ));
    }
    MILO_LOG("***********************************\n");
    DataPoint pt;
    static Symbol pid000("pid000");
    static Symbol pid001("pid001");
    if (profiles.size() > 1) {
        pt.AddPair(pid001, profiles[1]->GetOnlineID()->ToString());
    }
    pt.AddPair(pid000, profiles[0]->GetOnlineID()->ToString());
    SetDataPoint(pt);
}

void GetBadgeInfo(
    JsonConverter &c,
    const JsonObject *o,
    std::map<String, ChallengeBadgeInfo> &badgeInfos
) {
    JsonArray *a = const_cast<JsonArray *>(static_cast<const JsonArray *>(o));
    unsigned int aSize = a->GetSize();
    for (int i = 0; i < aSize; i++) {
        JsonArray *cur = static_cast<JsonArray *>(c.GetValue(a, i));
        String gamerTag = c.GetValue(cur, 0)->Str();
        int hmxGold = c.GetValue(cur, 3)->Int();
        int hmxSilver = c.GetValue(cur, 2)->Int();
        int hmxBronze = c.GetValue(cur, 1)->Int();
        int dlcGold = c.GetValue(cur, 6)->Int();
        int dlcSilver = c.GetValue(cur, 5)->Int();
        int dlcBronze = c.GetValue(cur, 4)->Int();
        MILO_LOG("***********************************\n");
        MILO_LOG(MakeString(">>>>>>>>>> gamertag = %s\n", gamerTag.c_str()));
        MILO_LOG(MakeString(">>>>>>>>>> hmxGold = %i\n", hmxGold));
        MILO_LOG(MakeString(">>>>>>>>>> hmxSilver = %i\n", hmxSilver));
        MILO_LOG(MakeString(">>>>>>>>>> hmxBronze = %i\n", hmxBronze));
        MILO_LOG(MakeString(">>>>>>>>>> dlcGold = %i\n", dlcGold));
        MILO_LOG(MakeString(">>>>>>>>>> dlcSilver = %i\n", dlcSilver));
        MILO_LOG(MakeString(">>>>>>>>>> dlcBronze = %i\n", dlcBronze));
        MILO_LOG("***********************************\n");
        auto it = badgeInfos.find(gamerTag);
        if (it != badgeInfos.end()) {
            it->second.mMedalCounts[kBadgeGold] = dlcGold + hmxGold;
            it->second.mMedalCounts[kBadgeSilver] = dlcSilver + hmxSilver;
            it->second.mMedalCounts[kBadgeBronze] = dlcBronze + hmxBronze;
        } else {
            ChallengeBadgeInfo value;
            value.mMedalCounts[kBadgeGold] = dlcGold + hmxGold;
            value.mMedalCounts[kBadgeSilver] = dlcSilver + hmxSilver;
            value.mMedalCounts[kBadgeBronze] = dlcBronze + hmxBronze;
            badgeInfos[gamerTag] = value;
        }
    }
}

void GetChallengeBadgeCountsJob::GetBadgeInfo(
    std::map<String, ChallengeBadgeInfo> &badgeInfos
) {
    if (mResult == 1 && mJsonResponse) {
        ::GetBadgeInfo(mJsonReader, mJsonResponse, badgeInfos);
    }
}

bool TimeStampCmp(ChallengeRow r1, ChallengeRow r2) {
    return r1.mTimeStamp < r2.mTimeStamp;
}

void GetRows(
    JsonConverter &c,
    const JsonObject *o,
    std::map<String, std::vector<ChallengeRow> > &rows,
    bool &bref
) {
    std::map<String, std::vector<ChallengeRow> > calcedRows;
    JsonArray *a = const_cast<JsonArray *>(static_cast<const JsonArray *>(o));
    unsigned int aSize = a->GetSize();
    for (int i = 0; i < aSize; i++) {
        JsonArray *cur = static_cast<JsonArray *>(c.GetValue(a, i));
        ChallengeRow curRow;
        curRow.unk0 = c.GetValue(cur, 0)->Int();
        curRow.mGamertag = c.GetValue(cur, 1)->Str();
        curRow.mSongID = c.GetValue(cur, 2)->Int();
        curRow.mArtist = c.GetValue(cur, 3)->Str();
        curRow.mSongTitle = c.GetValue(cur, 4)->Str();
        curRow.mScore = c.GetValue(cur, 5)->Int();
        curRow.mDiff = c.GetValue(cur, 6)->Int();
        curRow.mType = ChallengeRow::kNumChallengeTypes;
        curRow.unk2c = c.GetValue(cur, 7)->Str();
        DateTime dt;
        dt.ParseDate(c.GetValue(cur, 8)->Str());
        curRow.mTimeStamp = dt.ToCode();
        curRow.mChallengerXp = c.GetValue(cur, 9)->Int();
        calcedRows[curRow.unk2c].push_back(curRow);
    }
    bref = false;
    FOREACH (it, calcedRows) {
        std::sort(it->second.begin(), it->second.end(), TimeStampCmp);
        if (!bref) {
            auto rowIt = rows.find(it->first);
            if (rowIt != rows.end()) {
                std::vector<ChallengeRow> &rowItRows = rowIt->second;
                std::vector<ChallengeRow> &itRows = it->second;
                if (rowItRows.size() == itRows.size()) {
                    for (int i = 0; i < it->second.size(); i++) {
                        if (rowItRows[i] != itRows[i]) {
                            bref = true;
                            break;
                        }
                    }
                }
            } else {
                bref = true;
            }
        }
    }
    rows.swap(calcedRows);
}

void GetPlayerChallengesJob::GetRows(
    std::map<String, std::vector<ChallengeRow> > &challengeRows, bool &bref
) {
    if (mResult == 1 && mJsonResponse) {
        ::GetRows(mJsonReader, mJsonResponse, challengeRows, bref);
    }
}
