#pragma once
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/ChallengeSystemJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"

class Challenges : public Hmx::Object {
public:
    Challenges();
    virtual ~Challenges();
    virtual DataNode Handle(DataArray *, bool);

    static void Init();

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

protected:
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

    bool unk2c;
    unsigned int unk30;
    GetOfficialChallengesJob *unk34; // 0x34
    std::map<String, std::vector<ChallengeRow> > unk38;
    std::vector<ChallengeRow> unk50;
    Timer unk60;
    Timer unk90;
    int mScoreFactorDenom; // 0xc0
    std::vector<int> mSongTierFactor; // 0xc4
    int mConsolationXP; // 0xd0
    int unkd4;
    double unkd8;
    bool unke0;
    bool unke1;
    int unke4;
    std::list<FlauntStatusData> unke8;
    std::list<HamProfile *> unkf0;
    FlauntScoreData unkf8;
    std::vector<String> unk104;
    bool unk110;
    std::vector<ChallengeRow> unk114[2];
    int unk12c;
    std::map<String, ChallengeBadgeInfo> unk130;
    std::vector<int> mExportedDC1SongIDs; // 0x148
    std::vector<int> mExportedDC2SongIDs; // 0x154
};

extern Challenges *TheChallenges;
