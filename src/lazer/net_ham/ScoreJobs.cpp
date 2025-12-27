#include "net_ham/ScoreJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"

RecordScoreJob::RecordScoreJob(
    Hmx::Object *callback, RecordScoreData &data, int songID, bool provideInstarank
)
    : RCJob("leaderboards/recordscore/", callback) {
    DataPoint pt;
    static Symbol song_id("song_id");
    static Symbol provide_instarank("provide_instarank");
    static Symbol xp("xp");
    char buffer[36];
    Hx_snprintf(buffer, 36, "score%03d", 0);
    pt.AddPair(buffer, data.mStatus->mScore);
    Hx_snprintf(buffer, 36, "coop_score%03d", 0);
    pt.AddPair(buffer, data.mStatus->mCoopScore);
    Hx_snprintf(buffer, 36, "stars%03d", 0);
    pt.AddPair(buffer, data.mStatus->mStars);
    Hx_snprintf(buffer, 36, "no_flashcards%03d", 0);
    pt.AddPair(buffer, data.mStatus->mNoFlashcards != false);
    Hx_snprintf(buffer, 36, "diff%03d", 0);
    pt.AddPair(buffer, data.mStatus->mDifficulty);
    Hx_snprintf(buffer, 36, "pid%03d", 0);
    pt.AddPair(buffer, data.mProfile ? data.mProfile->GetOnlineID()->ToString() : "N/A");
    Hx_snprintf(buffer, 36, "xp%03d", 0);
    pt.AddPair(buffer, data.mProfile ? data.mProfile->GetMetagameRank()->RankNumber() : 0);
    Hx_snprintf(buffer, 36, "c_score%03d", 0);
    pt.AddPair(buffer, data.unkc);
    Hx_snprintf(buffer, 36, "cc_score%03d", 0);
    pt.AddPair(buffer, data.unk10);
    float f1 = 0;
    float f2 = 0;
    float f3 = 0;
    data.mProfile->GetFitnessStats(f1, f2, f3);
    Hx_snprintf(buffer, 36, "calories%03d", 0);
    pt.AddPair(buffer, f3);
    pt.AddPair(song_id, songID);
    pt.AddPair(provide_instarank, provideInstarank != false);
    SetDataPoint(pt);
}
