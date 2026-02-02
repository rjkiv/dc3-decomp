#include "meta_ham/Challenges.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/ChallengeSortByScore.h"
#include "meta_ham/ChallengeSortMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/PassiveMessenger.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SaveLoadManager.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/ChallengeSystemJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "stl/_algo.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Challenges *TheChallenges;

bool ChallengeScoreCmp(ChallengeRow cRow1, ChallengeRow cRow2) {
    return (unsigned int)cRow1.mScore < (unsigned int)cRow2.mScore;
}

Challenges::Challenges() {
    mFlauntingProfile = nullptr;
    mHasFlaunted = false;
    unk2c = false;
    mGetPlayerChallengesJob = nullptr;
    mGetOfficialChallengesJob = nullptr;
    mGetChallengeBadgeCountsJob = nullptr;
    unkd8 = -1;
    mOfficialChallengesDirty = false;
    mPlayerChallengesDirty = false;
    SetName("challenges", ObjectDir::Main());
    static Symbol udpate_duration("udpate_duration");
    static Symbol auto_update_duration("auto_update_duration");
    static Symbol xp_calculation("xp_calculation");
    static Symbol score_factor_denominator("score_factor_denominator");
    static Symbol song_tier_factor("song_tier_factor");
    static Symbol consolation_xp("consolation_xp");
    DataArray *cfg = SystemConfig("challenges", "config");
    DataArray *xpArr = cfg->FindArray(xp_calculation);
    mScoreFactorDenom = xpArr->FindInt(score_factor_denominator);
    mConsolationXP = xpArr->FindInt(consolation_xp);
    DataArray *tierArr = xpArr->FindArray(song_tier_factor);
    for (int i = 1; i < tierArr->Size(); i++) {
        tierArr->Array(i)->Int(1);
        mSongTierFactor.push_back(tierArr->Array(i)->Int(1));
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
    HANDLE_ACTION(clear_flaunt_flag, mFlauntedProfiles.clear())
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
    return !mGetPlayerChallengesJob && !mPlayerChallengeTimer.Running();
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

void Challenges::DownloadOfficialChallenges() {
    if (mGetOfficialChallengesJob) {
        mGetOfficialChallengesJob->Cancel(false);
        mGetOfficialChallengesJob = nullptr;
    }
    mGetOfficialChallengesJob = new GetOfficialChallengesJob(this);
    TheRockCentral.ManageJob(mGetOfficialChallengesJob);
}

int Challenges::GetGlobalChallengeSongID() {
    for (int i = 0; i < mOfficialChallenges.size(); i++) {
        if (mOfficialChallenges[i].IsHMXChallenge()) {
            return mOfficialChallenges[i].mSongID;
        }
    }
    return 0;
}

int Challenges::GetDlcChallengeSongID() {
    for (int i = mOfficialChallenges.size() - 1; i >= 0; i--) {
        if (mOfficialChallenges[i].IsDLCChallenge()) {
            return mOfficialChallenges[i].mSongID;
        }
    }
    return 0;
}

String Challenges::GetGlobalChallengeSongName() {
    for (int i = 0; i < mOfficialChallenges.size(); i++) {
        if (mOfficialChallenges[i].IsHMXChallenge()) {
            return mOfficialChallenges[i].mSongTitle;
        }
    }
    return gNullStr;
}

String Challenges::GetDlcChallengeSongName() {
    for (int i = mOfficialChallenges.size() - 1; i >= 0; i--) {
        if (mOfficialChallenges[i].IsDLCChallenge()) {
            return mOfficialChallenges[i].mSongTitle;
        }
    }
    return gNullStr;
}

int Challenges::CalculateChallengeXp(int score, int i2) {
    int rankTier = TheHamSongMgr.RankTier(i2);
    MILO_ASSERT(rankTier >= 0 && rankTier < mSongTierFactor.size(), 0x1CA);
    return (score / mScoreFactorDenom) + mSongTierFactor[rankTier];
}

bool Challenges::HasFlaunted(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2D0);
    for (int i = 0; i < mFlauntedProfiles.size(); i++) {
        if (mFlauntedProfiles[i] == profile->GetName()) {
            return true;
        }
    }
    return false;
}

String Challenges::GetMissionInfoGamertag() {
    static Symbol primary_challenge_player("primary_challenge_player");
    static Symbol challenge_mission_index("challenge_mission_index");
    int playerIndex = TheHamProvider->Property(primary_challenge_player)->Int();
    HamPlayerData *playerData = TheGameData->Player(playerIndex);
    MILO_ASSERT(playerData, 0x37F);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x381);
    int challengeIndex = provider->Property(challenge_mission_index)->Int();
    return mPlayerChallenges[playerIndex][challengeIndex].mGamertag;
}

int Challenges::GetMissionInfoScore() {
    static Symbol primary_challenge_player("primary_challenge_player");
    static Symbol challenge_mission_index("challenge_mission_index");
    int playerIndex = TheHamProvider->Property(primary_challenge_player)->Int();
    HamPlayerData *playerData = TheGameData->Player(playerIndex);
    MILO_ASSERT(playerData, 0x390);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x392);
    int challengeIndex = provider->Property(challenge_mission_index)->Int();
    return mPlayerChallenges[playerIndex][challengeIndex].mScore;
}

void Challenges::UpdateInGameEvent() {
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    static Symbol num_challenge_targets("num_challenge_targets");
    static Symbol challenge_target_index("challenge_target_index");
    static Symbol challenge_target_score("challenge_target_score");
    static Symbol challenge_target_rival("challenge_target_rival");
    static Symbol score("score");
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x3A5);
        PropertyEventProvider *provider = playerData->Provider();
        MILO_ASSERT(provider, 0x3A7);
        if (provider->Property(has_valid_challenge_data)->Int()) {
            int i6 = provider->Property(challenge_target_index)->Int();
            int i7 = provider->Property(score)->Int();
            int i8 = provider->Property(challenge_target_score)->Int();
            int i9 = provider->Property(num_challenge_targets)->Int();
            if (i6 < i9 && i7 > i8) {
                i6++;
                if (i6 >= i9) {
                    provider->SetProperty(challenge_target_index, i9);
                    provider->SetProperty(challenge_target_score, 0);
                } else {
                    provider->SetProperty(challenge_target_index, i6);
                    provider->SetProperty(
                        challenge_target_score, mPlayerChallenges[i][i6].mScore
                    );
                    provider->SetProperty(
                        challenge_target_rival, mPlayerChallenges[i][i6].mGamertag
                    );
                }
            }
        }
    }
}

void Challenges::ResetInGameEvent() {
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    static Symbol num_challenge_targets("num_challenge_targets");
    static Symbol challenge_target_index("challenge_target_index");
    static Symbol challenge_target_score("challenge_target_score");
    static Symbol challenge_target_rival("challenge_target_rival");
    static Symbol score("score");
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x3D7);
        PropertyEventProvider *provider = playerData->Provider();
        MILO_ASSERT(provider, 0x3D9);
        if (provider->Property(has_valid_challenge_data)->Int()) {
            provider->SetProperty(challenge_target_index, 0);
            provider->SetProperty(challenge_target_score, mPlayerChallenges[i][0].mScore);
            provider->SetProperty(
                challenge_target_rival, mPlayerChallenges[i][0].mGamertag
            );
        }
    }
}

void Challenges::PollInGameStatus() {
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    static Symbol player_name("player_name");
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x3EC);
        PropertyEventProvider *provider = playerData->Provider();
        MILO_ASSERT(provider, 0x3EE);
        if (provider->Property(has_valid_challenge_data)->Int()) {
            HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
            if (profile) {
                if (mPlayerChallenges[i].size() != 0) {
                    if ((unsigned int)mPlayerChallenges[i][0].mType
                        == ChallengeRow::kNumChallengeTypes) {
                        if (mPlayerChallenges[i][0].unk2c
                            != provider->Property(player_name)->Str()) {
                            provider->SetProperty(has_valid_challenge_data, false);
                        }
                    }
                } else {
                    provider->SetProperty(has_valid_challenge_data, false);
                }
            } else {
                provider->SetProperty(has_valid_challenge_data, false);
            }
        }
    }
}

int Challenges::GetTotalXpEarned(int player) {
    HamPlayerData *playerData = TheGameData->Player(player);
    MILO_ASSERT(playerData, 0x436);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x438);
    static Symbol score("score");
    int playerScore = provider->Property(score)->Int();
    std::vector<ChallengeRow> &playerChallenges = mPlayerChallenges[player];
    int xp = 0;
    for (int i = 0; i < playerChallenges.size(); i++) {
        if (playerScore > playerChallenges[i].mScore) {
            xp += CalculateChallengeXp(
                playerChallenges[i].mScore, playerChallenges[i].mDiff
            );
        }
    }
    if (xp != 0) {
        return xp;
    } else
        return mConsolationXP;
}

void Challenges::UploadNextFlaunt() {
    if (!mHasFlaunted) {
        mHasFlaunted = true;
    }
    mFlauntScoreData.mStatus = &mFlauntList.front();
    MILO_LOG("***********************************\n");
    MILO_LOG("Challenges::UploadNextFlaunt()\n");
    MILO_LOG(MakeString("   mDiff       = %d\n", mFlauntScoreData.mStatus->mDiff));
    MILO_LOG(MakeString("   mNeedUpload = %d\n", mFlauntScoreData.mStatus->mNeedUpload));
    MILO_LOG(MakeString("   mScore      = %d\n", mFlauntScoreData.mStatus->mScore));
    MILO_LOG(MakeString("   mSongID     = %d\n", mFlauntScoreData.mStatus->mSongID));
    MILO_LOG("***********************************\n");
    mFlauntScoreData.mProfile = mFlauntingProfile;
    TheRockCentral.ManageJob(new FlauntScoreJob(this, mFlauntScoreData));
}

void Challenges::ReadPlayerChallengesComplete(bool b1) {
    mPlayerChallengesDirty = false;
    mGetPlayerChallengesJob->GetRows(mProfileChallenges, mPlayerChallengesDirty);
    mGetPlayerChallengesJob = nullptr;
    static Message allUpdatedMsg("all_challenges_updated", 0);
    static Message challengesFailedRcMsg("challenges_failed_rc");
    static Message challengesFailedLiveMsg("challenges_failed_live");
    if (b1) {
        if (!mGetOfficialChallengesJob) {
            bool dirty = mPlayerChallengesDirty || mOfficialChallengesDirty;
            allUpdatedMsg[0] = dirty;
            TheUI->Handle(allUpdatedMsg, true);
            mPlayerChallengesDirty = false;
            mOfficialChallengesDirty = false;
        }
    } else if (ThePlatformMgr.IsConnected()) {
        TheUI->Handle(challengesFailedRcMsg, b1);
    } else {
        TheUI->Handle(challengesFailedLiveMsg, b1);
    }
    mPlayerChallengeTimer.Restart();
}

void Challenges::ReadOfficialChallengesComplete(bool b1) {
    mOfficialChallengesDirty = false;
    mGetOfficialChallengesJob->GetRows(
        mOfficialChallenges, unkd8, mOfficialChallengesDirty
    );
    mGetOfficialChallengesJob = nullptr;
    static Message allUpdatedMsg("all_challenges_updated", 0);
    static Message challengesFailedRcMsg("challenges_failed_rc");
    static Message challengesFailedLiveMsg("challenges_failed_live");
    if (b1) {
        if (!mGetPlayerChallengesJob) {
            bool dirty = mPlayerChallengesDirty || mOfficialChallengesDirty;
            allUpdatedMsg[0] = dirty;
            TheUI->Handle(allUpdatedMsg, true);
            mPlayerChallengesDirty = false;
            mOfficialChallengesDirty = false;
        }
    } else {
        if (ThePlatformMgr.IsConnected()) {
            TheUI->Handle(challengesFailedRcMsg, b1);
        } else {
            TheUI->Handle(challengesFailedLiveMsg, b1);
        }
        mOfficialChallengeTimer.Restart();
    }
}

void Challenges::UpdateChallengeTimeStamp() {
    if (mProfileChallenges.size()) {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            FOREACH (it, mProfileChallenges) {
                if (it->first == profile->GetName()) {
                    MILO_LOG(
                        ">>>> Update challenge time stamp from %i to %i\n",
                        profile->GetUnk324(),
                        it->second[0].mTimeStamp
                    );
                    profile->MakeDirty();
                    profile->SetUnk324(it->second[0].mTimeStamp);
                    return;
                }
            }
        }
    }
}

void Challenges::AddPendingProfile(HamProfile *profile) {
    MILO_ASSERT(profile, 0x11F);
    bool found = false;
    FOREACH (it, mPendingProfiles) {
        if (*it == profile) {
            found = true;
            break;
        }
    }
    if (!found) {
        mPendingProfiles.push_back(profile);
    }
}

void Challenges::StartUploadingNextProfile() {
    if (!mPendingProfiles.empty()) {
        mFlauntingProfile = mPendingProfiles.front();
        mPendingProfiles.pop_front();
        mFlauntingProfile->GetSongStatusMgr()->GetFlauntsToUpload(mFlauntList);
        MILO_ASSERT(!mFlauntList.empty(), 0x155);
        UploadNextFlaunt();
    }
}

int Challenges::GetMedalCount(int type) {
    MILO_ASSERT(type >= 0 && type < kNumBadgeTypes, 0x1D2);
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        String name = profile->GetName();
        auto it = mProfileBadgeInfos.find(name);
        if (it != mProfileBadgeInfos.end()) {
            return it->second.mMedalCounts[type];
        }
    }
    return 0;
}

bool Challenges::NeedToReSyncChallenges() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x454);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profile) {
            String name = profile->GetName();
            auto it = mProfileChallenges.find(name);
            if (it == mProfileChallenges.end()) {
                return true;
            }
        }
    }
    return false;
}

bool Challenges::GetBeatenChallengeXPs(
    const HamPlayerData *playerData, int score, std::vector<int> &beatenXPs
) {
    MILO_ASSERT(playerData, 0x40C);
    PropertyEventProvider *provider = playerData->Provider();
    MILO_ASSERT(provider, 0x40E);
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    beatenXPs.clear();
    if (provider->Property(has_valid_challenge_data)->Int()) {
        for (int i = 0; i < 2; i++) {
            if (playerData == TheGameData->Player(i)) {
                std::vector<ChallengeRow> &challenges = mPlayerChallenges[i];
                if (challenges.size() == 0)
                    return false;
                for (int j = 0; j < challenges.size(); j++) {
                    if (score > challenges[j].mScore) {
                        int xp = CalculateChallengeXp(
                            challenges[j].mScore, challenges[j].mDiff
                        );
                        beatenXPs.push_back(xp);
                        MILO_LOG("XP = %i\n", xp);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

void Challenges::GetPlayerChallenges(std::vector<ChallengeRow> &rows) {
    if (mProfileChallenges.size()) {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            FOREACH (it, mProfileChallenges) {
                if (it->first == profile->GetName()) {
                    rows = it->second;
                }
            }
        }
    }
}

void Challenges::GetOfficialChallenges(std::vector<ChallengeRow> &rows) {
    rows = mOfficialChallenges;
}

void Challenges::DownloadBadgeInfo() {
    if (mGetChallengeBadgeCountsJob) {
        mGetChallengeBadgeCountsJob->Cancel(false);
        mGetChallengeBadgeCountsJob = nullptr;
    }
    std::vector<HamProfile *> profiles;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x48C);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profile) {
            profile->UpdateOnlineID();
            if (profile->IsSignedIn()
                && ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
                profiles.push_back(profile);
            }
        }
    }
    if (profiles.size() != 0) {
        mGetChallengeBadgeCountsJob = new GetChallengeBadgeCountsJob(this, profiles);
        TheRockCentral.ManageJob(mGetChallengeBadgeCountsJob);
    } else {
        mProfileBadgeInfos.clear();
        mGetChallengeBadgeCountsJob = nullptr;
        static Message badgeUpdatedMsg("player_badge_count_updated");
        TheUI->Handle(badgeUpdatedMsg, false);
    }
}

void Challenges::ReadBadgeInfo(bool b1) {
    GetChallengeBadgeCountsJob *job = mGetChallengeBadgeCountsJob;
    mProfileBadgeInfos.clear();
    job->GetBadgeInfo(mProfileBadgeInfos);
    mGetChallengeBadgeCountsJob = nullptr;
    static Message badgeUpdatedMsg("player_badge_count_updated");
    static Message challengesFailedRcMsg("challenges_failed_rc");
    static Message challengesFailedLiveMsg("challenges_failed_live");
    if (b1) {
        TheUI->Handle(badgeUpdatedMsg, b1);
    } else if (ThePlatformMgr.IsConnected()) {
        TheUI->Handle(challengesFailedRcMsg, b1);
    } else {
        TheUI->Handle(challengesFailedLiveMsg, b1);
    }
}

void Challenges::UploadFlaunt(HamProfile *profile, bool b2) {
    profile->UpdateOnlineID();
    if (profile->IsSignedIn() && ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
        std::list<FlauntStatusData> flaunts;
        profile->GetSongStatusMgr()->GetFlauntsToUpload(flaunts);
        if (flaunts.empty()) {
            return;
        } else {
            if (mFlauntingProfile && mFlauntingProfile != profile) {
                AddPendingProfile(profile);
            } else {
                mFlauntingProfile = profile;
                mFlauntList = flaunts;
                UploadNextFlaunt();
            }
            if (!b2) {
                mFlauntedProfiles.push_back(String(profile->GetName()));
            }
        }
    }
}

void Challenges::UploadFlauntForOne() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(profile, 0x65);
    profile->UpdateFlaunt();
    UploadFlaunt(profile, false);
}

void Challenges::UploadFlauntForAll(bool b1) {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x70);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profile) {
            UploadFlaunt(profile, b1);
        }
    }
}

void Challenges::Init() {
    MILO_ASSERT(!TheChallenges, 0x23);
    TheChallenges = new Challenges();
}

void Challenges::SetupInGameData() {
    NavListSortNode *node = TheChallengeSortMgr->GetHighlightItem();
    MILO_ASSERT(node, 0x2e5);

    // stuff

    HamProfile *primaryProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(primaryProfile, 0x2f0);
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    static Symbol primary_challenge_player("primary_challenge_player");
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x2fb);
        PropertyEventProvider *provider = playerData->Provider();
        MILO_ASSERT(provider, 0x2fd);
        provider->SetProperty(0, 0);
        HamProfile *profileFromPad =
            TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profileFromPad) {
            if (profileFromPad == primaryProfile) {
                SetupInGameChallenges(
                    0, 0, 0, primaryProfile, true, mPlayerChallenges[2], provider
                );
                TheHamProvider->SetProperty(primary_challenge_player, 0);
            } else {
                SetupInGameChallenges(
                    0, 0, 0, profileFromPad, false, mPlayerChallenges[2], provider
                );
            }
        }
    }
}

bool Challenges::NotRunning() {
    return !mGetPlayerChallengesJob && !mPlayerChallengeTimer.Running();
}

void Challenges::AutoDownloadPlayerChallenges() {
    UIPanel *mainPanel = ObjectDir::Main()->Find<UIPanel>("main_panel");
    UIPanel *challengeFeedPanel =
        ObjectDir::Main()->Find<UIPanel>("challenge_feed_panel");
    if (mainPanel->GetState() == UIPanel::kUp
        || challengeFeedPanel->GetState() == UIPanel::kUp) {
        if (NotRunning()) {
            DownloadPlayerChallenges();
        }
    }
}

void Challenges::SetupInGameChallenges(
    int i1,
    int i2,
    char const *c,
    HamProfile *profile,
    bool b,
    std::vector<ChallengeRow> &challengeRows,
    PropertyEventProvider *provider
) {
    MILO_ASSERT(profile, 0x31c);
    static Symbol num_challenge_targets("num_challenge_targets");
    static Symbol challenge_target_index("challenge_target_index");
    static Symbol challenge_mission_index("challenge_mission_index");
    static Symbol challenge_target_score("challenge_target_score");
    static Symbol has_valid_challenge_data("has_valid_challenge_data");
    static Symbol challenge_target_rival("challenge_target_rival");
    static Symbol challenge_mission_score("challenge_mission_score");
    static Symbol is_challenging_self("is_challenging_self");

    int songID = 2;
    if (i1 == GetGlobalChallengeSongID()) {
        songID = 0;
    } else if (i1 == GetDlcChallengeSongID()) {
        songID = 1;
    }

    String profileName = profile->GetName();
    auto it = mProfileChallenges.find(profileName);
    if (it != mProfileChallenges.end()) {
        const std::vector<ChallengeRow> &vec = it->second;
        for (int i = 0; i < vec.size(); i++) {
            if (vec[i].mSongID == i1) {
                challengeRows.push_back(vec[i]);
            }
        }
    } else {
        if (songID == 2 && b) {
            MILO_ASSERT(0, 0x343);
        }
    }

    for (int i = 0; i < mOfficialChallenges.size(); i++) {
        if (mOfficialChallenges[i].mSongID == i1) {
            challengeRows.push_back(mOfficialChallenges[i]);
        }
    }

    if (challengeRows.empty()) {
        return;
    }

    std::sort(challengeRows.begin(), challengeRows.end(), ChallengeScoreCmp);
    MILO_LOG(">>>>>>>>>> %s in game data\n", b ? "Primary" : "2nd");

    for (int i = 0; i < challengeRows.size(); i++) {
        int score = challengeRows[i].mScore;
        MILO_LOG(
            ">>>>>>>>>> score = %i, gamertag = %s\n", score, challengeRows[i].mGamertag
        );
    }
    provider->SetProperty(num_challenge_targets, (int)challengeRows.size());
    provider->SetProperty(challenge_target_index, 0);
    provider->SetProperty(challenge_target_score, challengeRows.front().mScore);
    provider->SetProperty(challenge_target_rival, challengeRows.front().mGamertag);
    provider->SetProperty(has_valid_challenge_data, true);
    provider->SetProperty(is_challenging_self, false);

    for (int i = 0; i < challengeRows.size(); i++) {
        if (challengeRows[i].mSongID == i1 && challengeRows[i].mScore == i2
            && challengeRows[i].mGamertag == c) {
            if (profileName == c) {
                provider->SetProperty(is_challenging_self, true);
            }
            provider->SetProperty(challenge_mission_index, i);
            provider->SetProperty(challenge_mission_score, challengeRows[i].mScore);
            break;
        }
    }
}

DataNode Challenges::OnMsg(const RCJobCompleteMsg &msg) {
    if (msg.Job() == mGetPlayerChallengesJob) {
        ReadPlayerChallengesComplete(msg.Success() != 0);
    } else if (msg.Job() == mGetOfficialChallengesJob) {
        ReadOfficialChallengesComplete(msg.Success() != 0);
    } else if (msg.Job() == mGetChallengeBadgeCountsJob) {
        ReadBadgeInfo(msg.Success() != 0);
    } else {
        if (msg.Success()) {
            if (mFlauntingProfile) {
                static Symbol p1("p1");
                static Symbol p2("p2");
                static Symbol challenges_regular_flaunt_sent(
                    "challenges_regular_flaunt_sent"
                );
                static Symbol side("side");
                bool inGameData = false;
                for (int i = 0; i < 2; i++) {
                    HamPlayerData *playerData = TheGameData->Player(i);
                    MILO_ASSERT(playerData, 0x1fe);
                    PropertyEventProvider *provider = playerData->Provider();
                    MILO_ASSERT(provider, 0x200);
                    if (playerData->PadNum() == mFlauntingProfile->GetPadNum()) {
                        const DataNode *sideNode = provider->Property(side, true);
                        Symbol triggerMsg = sideNode->Int() == 0 ? p2 : p1;
                        mFlauntingProfile->IncrementFlauntCount();
                        if (5 <= mFlauntingProfile->GetFlauntCount()) {
                            static Symbol acc_flaunt("acc_flaunt");
                            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                                mFlauntingProfile, acc_flaunt, false
                            );
                        }
                        ThePassiveMessenger->TriggerGenericMsg(
                            challenges_regular_flaunt_sent,
                            triggerMsg,
                            (PassiveMessageType)0,
                            gNullStr,
                            -1
                        );
                        inGameData = true;
                        break;
                    }
                }
                if (!inGameData) {
                    MILO_LOG(
                        "A score is flaunted but the associated profile is not in game data, what happened?\n"
                    );
                }
            } else {
                MILO_LOG(
                    "A score is flaunted but the associated profile is NULL, wait what?\n"
                );
            }
            if (!mFlauntList.empty()) {
                FlauntStatusData data = mFlauntList.front();
                if (mFlauntingProfile) {
                    SongStatusMgr *mgr = mFlauntingProfile->GetSongStatusMgr();
                    mgr->ClearFlauntsNeedUpload(data.mSongID);
                    mFlauntingProfile->MakeDirty();
                    if (TheSaveLoadMgr) {
                        TheSaveLoadMgr->AutoSave();
                    }
                }
                mFlauntList.pop_front();
                if (!mFlauntList.empty()) {
                    UploadNextFlaunt();

                } else {
                    if (!mPendingProfiles.empty()) {
                        StartUploadingNextProfile();
                    } else {
                        mFlauntingProfile = nullptr;
                        mHasFlaunted = false;
                        DataNode score("score");
                        DataNode updated("updated");
                        ThePlatformMgr.SmartGlassSend(0, DataArrayPtr(updated, score));
                    }
                }
            }

        } else {
            MILO_LOG(
                "Failed to flaunt for %s\n",
                mFlauntingProfile ? mFlauntingProfile->GetName() : "N/A"
            );
            mPendingProfiles.clear();
            mFlauntingProfile = nullptr;
            mHasFlaunted = false;
        }
    }
    return 1;
}

bool Challenges::ChallengesDirty() {
    return mPlayerChallengesDirty || mOfficialChallengesDirty;
}

void Challenges::DownloadPlayerChallenges() {
    if (mPlayerChallengeTimer.Running()) {
        mPlayerChallengeTimer.Stop();
    }

    if (mHasFlaunted) {
        unk2c = true;
    } else {
        if (mGetPlayerChallengesJob) {
            mGetPlayerChallengesJob->Cancel(false);
            mGetPlayerChallengesJob = nullptr;
        }
        std::vector<HamProfile *> profiles;
        for (int i = 0; i < 2; i++) {
            HamPlayerData *playerData = TheGameData->Player(i);
            MILO_ASSERT(playerData, 0xed);
            HamProfile *profileFromPad =
                TheProfileMgr.GetProfileFromPad(playerData->PadNum());
            if (profileFromPad) {
                profileFromPad->UpdateOnlineID();
                if (profileFromPad->IsSignedIn()) {
                    int padNum = profileFromPad->GetPadNum();
                    if (ThePlatformMgr.IsSignedIntoLive(padNum)) {
                        profiles.push_back(profileFromPad);
                    }
                }
            }
        }
        if (profiles.size() != 0) {
            mGetPlayerChallengesJob = new GetPlayerChallengesJob(this, profiles);
            TheRockCentral.ManageJob(mGetPlayerChallengesJob);
        } else {
            mProfileChallenges.clear();
            mGetPlayerChallengesJob = nullptr;
            static Message all_challenges_updated("all_challenges_updated", 0);
            if (!mGetOfficialChallengesJob) {
                all_challenges_updated[0] = ChallengesDirty();
                TheUI->Handle(all_challenges_updated, true);
                mPlayerChallengesDirty = false;
                mOfficialChallengesDirty = false;
            }
            mPlayerChallengeTimer.Restart();
        }
    }
}

void Challenges::Poll() {
    if (mOfficialChallengeTimer.Running()) {
        if (1.0f <= mOfficialChallengeTimer.SplitMs() / TheRockCentral.GetUnk84()) {
            mOfficialChallengeTimer.Stop();
            DownloadOfficialChallenges();
        }
    }

    if (unk2c) {
        if (!mHasFlaunted) {
            unk2c = false;
            DownloadPlayerChallenges();
            goto jump;
        }
    }

    if (mPlayerChallengeTimer.Running()) {
        if (1.0f <= mPlayerChallengeTimer.SplitMs() / TheRockCentral.GetUnk84()) {
            mPlayerChallengeTimer.Stop();
            AutoDownloadPlayerChallenges();
        }
    }
jump:
    if (unkd8 != -1.0) {
        unkd8 -= TheTaskMgr.DeltaUISeconds();
        if (unkd8 < 0) {
            unkd8 = 0;
            UIPanel *challengeFeedPanel =
                ObjectDir::Main()->Find<UIPanel>("challenge_feed_panel");
            if (challengeFeedPanel->GetState() == 1) {
                static Message challenges_expired("challenges_expired");
                TheUI->Handle(challenges_expired, true);
            } else {
                DownloadOfficialChallenges();
                DownloadPlayerChallenges();
            }
        }
    }
}
