#include "meta_ham/Challenges.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/System.h"

Challenges *TheChallenges;

Challenges::Challenges() {
    unk2c = false;
    unk110 = false;
    unk12c = 0;
    unke0 = false;
    unke4 = 0;
    unkd8 = -1;
    unk30 = 0;
    unk34 = 0;
    unke1 = false;
    SetName("challenges", ObjectDir::Main());
    static Symbol udpate_duration("udpate_duration");
    static Symbol auto_update_duration("auto_update_duration");
    static Symbol xp_calculation("xp_calculation");
    static Symbol score_factor_denominator("score_factor_denominator");
    static Symbol song_tier_factor("song_tier_factor");
    static Symbol consolation_xp("consolation_xp");
    DataArray *cfg = SystemConfig("challenges", "config");
    DataArray *xpArr = cfg->FindArray(xp_calculation);
    unkc0 = xpArr->FindInt(score_factor_denominator);
    unkd0 = xpArr->FindInt(consolation_xp);
    DataArray *tierArr = xpArr->FindArray(song_tier_factor);
    for (int i = 1; i < tierArr->Size(); i++) {
        tierArr->Array(i)->Int(1);
        unkc4.push_back(tierArr->Array(i)->Int(1));
    }
    DataArray *dc1Arr = cfg->FindArray("exported_songids_dc1");
    for (int i = 1; i < dc1Arr->Size(); i++) {
        mExportedDC1SongIDs.push_back(dc1Arr->Int(i));
    }
    DataArray *dc2Arr = cfg->FindArray("exported_songids_dc2");
    for (int i = 1; i < dc2Arr->Size(); i++) {
        mExportedDC2SongIDs.push_back(dc2Arr->Int(i));
    }
}

Challenges::~Challenges() {}

BEGIN_HANDLERS(Challenges)
    HANDLE_ACTION(clear_flaunt_flag, unk104.clear())
    HANDLE_EXPR(has_flaunted, HasFlaunted(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(download_player_challenges, DownloadPlayerChallenges())
    HANDLE_ACTION(update_challenge_timestamp, UpdateChallengeTimeStamp())
    HANDLE_ACTION(upload_flaunt_for_one, UploadFlauntForOne())
    HANDLE_ACTION(upload_flaunt_for_all, UploadFlauntForAll(false))
    HANDLE_ACTION(setup_in_game_data, SetupInGameData())
    HANDLE_EXPR(get_challenge_mission_gamertag, GetMissionInfoGamertag())
    HANDLE_EXPR(get_challenge_mission_score, GetMissionInfoScore())
    HANDLE_ACTION(update_in_game_event, UpdateInGameEvent())
    HANDLE_ACTION(reset_in_game_event, ResetInGameEvent())
    HANDLE_ACTION(poll_in_game_status, PollInGameStatus())
    HANDLE_EXPR(get_total_xp_earned, GetTotalXpEarned(_msg->Int(2)))
    HANDLE_EXPR(can_download_player_challenges, CanDownloadPlayerChallenges())
    HANDLE_EXPR(need_to_resync_challenges, NeedToReSyncChallenges())
    HANDLE_ACTION(download_all_challenges, DownloadAllChallenges())
    HANDLE_ACTION(download_challenge_badge_info, DownloadBadgeInfo())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool Challenges::CanDownloadPlayerChallenges() const {
    return !unk30 && !unk60.Running();
}

bool Challenges::IsExportedSongDC1(int songID) {
    for (int i = 0; i < mExportedDC1SongIDs.size(); i++) {
        if (mExportedDC1SongIDs[i] == songID) {
            return true;
        }
    }
    return false;
}

bool Challenges::IsExportedSongDC2(int songID) {
    for (int i = 0; i < mExportedDC2SongIDs.size(); i++) {
        if (mExportedDC2SongIDs[i] == songID) {
            return true;
        }
    }
    return false;
}
