#pragma once
#include "hamobj/HamPlayerData.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/ChallengeSystemJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"

class ChallengeResultPanel;

class Challenges : public Hmx::Object {
    friend class ChallengeResultPanel;

public:
    Challenges();
    virtual ~Challenges();
    virtual DataNode Handle(DataArray *, bool);

    static void Init();

    void Poll();
    void DownloadPlayerChallenges();
    void UploadFlauntForOne();
    void UploadFlauntForAll(bool);
    int GetTotalXpEarned(int);
    void DownloadOfficialChallenges();
    void DownloadAllChallenges() {
        DownloadOfficialChallenges();
        DownloadPlayerChallenges();
    }
    bool CanDownloadPlayerChallenges() const;
    bool IsExportedSongDC1(int);
    bool IsExportedSongDC2(int);
    bool HasNewChallenges();
    int GetGlobalChallengeSongID();
    int GetDlcChallengeSongID();
    String GetGlobalChallengeSongName();
    String GetDlcChallengeSongName();
    int CalculateChallengeXp(int, int);
    int GetMedalCount(int);
    bool GetBeatenChallengeXPs(const HamPlayerData *, int, std::vector<int> &);
    void GetPlayerChallenges(std::vector<ChallengeRow> &);
    void GetOfficialChallenges(std::vector<ChallengeRow> &);
    void UploadFlaunt(HamProfile *, bool);
    bool GetExpireTime(int &, int &, int &, int &);

protected:
    void AddPendingProfile(HamProfile *);

    DataNode OnMsg(const RCJobCompleteMsg &);

private:
    bool HasFlaunted(HamProfile *);
    void UpdateChallengeTimeStamp();
    void SetupInGameData();
    String GetMissionInfoGamertag();
    int GetMissionInfoScore();
    void UpdateInGameEvent();
    void ResetInGameEvent();
    void PollInGameStatus();
    bool NeedToReSyncChallenges();
    void DownloadBadgeInfo();
    void UploadNextFlaunt();
    void ReadPlayerChallengesComplete(bool);
    void ReadOfficialChallengesComplete(bool);
    void StartUploadingNextProfile();
    void ReadBadgeInfo(bool);
    void SetupInGameChallenges(
        int,
        int,
        char const *,
        HamProfile *,
        bool,
        std::vector<ChallengeRow> &,
        PropertyEventProvider *
    );
    void AutoDownloadPlayerChallenges();
    bool NotRunning();
    bool ChallengesDirty();

    bool unk2c;
    GetPlayerChallengesJob *mGetPlayerChallengesJob; // 0x30
    GetOfficialChallengesJob *mGetOfficialChallengesJob; // 0x34
    // key = profile name, value = that player's challenges
    std::map<String, std::vector<ChallengeRow> > mProfileChallenges; // 0x38
    // official challenges? like, hmx's gold/silver/bronze challenge sets
    std::vector<ChallengeRow> mOfficialChallenges; // 0x50
    Timer mPlayerChallengeTimer; // 0x60
    Timer mOfficialChallengeTimer; // 0x90
    int mScoreFactorDenom; // 0xc0
    std::vector<int> mSongTierFactor; // 0xc4
    int mConsolationXP; // 0xd0
    double unkd8; // 0xd8
    /** Do our official challenges need updating/resyncing? */
    bool mOfficialChallengesDirty; // 0xe0
    /** Do our player challenges need updating/resyncing? */
    bool mPlayerChallengesDirty; // 0xe1
    HamProfile *mFlauntingProfile; // 0xe4
    std::list<FlauntStatusData> mFlauntList; // 0xe8
    std::list<HamProfile *> mPendingProfiles; // 0xf0
    FlauntScoreData mFlauntScoreData; // 0xf8
    // profile names that have flaunted
    std::vector<String> mFlauntedProfiles; // 0x104
    // gets marked true when uploading a flaunt - has flaunted?
    bool mHasFlaunted; // 0x110
    // challenge rows, 1 per player
    /** The series of live challenges each player has to beat. */
    std::vector<ChallengeRow> mPlayerChallenges[2]; // 0x114
    GetChallengeBadgeCountsJob *mGetChallengeBadgeCountsJob; // 0x12c
    // key = profile name, value = that player's badge info
    std::map<String, ChallengeBadgeInfo> mProfileBadgeInfos; // 0x130
    std::vector<int> mExportedDC1SongIDs; // 0x148
    std::vector<int> mExportedDC2SongIDs; // 0x154
};

extern Challenges *TheChallenges;
