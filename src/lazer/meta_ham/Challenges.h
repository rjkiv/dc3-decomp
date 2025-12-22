#pragma once
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"

// size 0x4c
class ChallengeRow {
public:
    int unk0;
    String unk4;
    int unkc;
    String unk10;
    String unk18;
    int unk20;
    int unk24;
    int unk28;
    String unk2c;
    int unk34;
    int unk38;
};

class ChallengeBadgeInfo {
public:
};

class FlauntScoreData {
public:
    FlauntScoreData() : unk4(0), unk8(0) {}
    virtual ~FlauntScoreData() {}
    int unk4;
    int unk8;
};

class Challenges : public Hmx::Object {
public:
    Challenges();
    virtual ~Challenges();
    virtual DataNode Handle(DataArray *, bool);

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
    int unk34; // 0x34 - GetOfficialChallengesJob*
    std::map<String, std::vector<ChallengeRow> > unk38;
    std::vector<ChallengeRow> unk50;
    Timer unk60;
    Timer unk90;
    int unkc0;
    std::vector<int> unkc4;
    int unkd0;
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
