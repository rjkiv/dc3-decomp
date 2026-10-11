#pragma once
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/LeaderboardJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/ScoreJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/Str.h"
#include "types.h"
#include "xdk/XAPILIB.h"

enum LeaderboardsType {
    kScore = 0x0000,
    kCoopScore = 0x0001,
    kCareerScore = 0x0002,
    kCappedCareerScore = 0x0003,
    kInfPlaylistScore = 0x0004,
    kInfPlaylistTime = 0x0005
};

enum LeaderboardsMode {
    kLeaderboardsTop = 0x0000,
    kLeaderboardsLocal = 0x0001,
    kLeaderboardsFriends = 0x0002
};

enum LeaderboardStatus {
    kLeaderboardStatusInactive = 0x0000,
    kLeaderboardStatusReady = 0x0001,
    kLeaderboardStatusPending = 0x0002,
    kLeaderboardStatusFailure = 0x0003,
    kLeaderboardStatusNoProfile = 0x0004,
    kLeaderboardStatusNotConnected = 0x0005
};

class Leaderboards : public Hmx::Object, public UIListProvider {
public:
    Leaderboards();
    // Hmx::Object
    virtual ~Leaderboards();
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

    void DownloadScores(Symbol);
    Symbol ShowGamercard(int, HamProfile *);
    void SetType(int);
    void SetMode(int);
    bool HasSelf() const;
    bool IsSelf(int) const;
    void ClearCache();
    void Poll();
    void UploadScores(HamProfile *);
    void ReadScoresComplete(bool, bool);

    static void Init();

private:
    void GetScores(int);
    void UploadNextScore();
    void PostProcScores();
    void StartUploadingNextProfile();

protected:
    void AddPendingProfile(HamProfile *);

    DataNode OnMsg(const RCJobCompleteMsg &);

private:
    std::list<SongStatusData> mScoreList; // 0x30
    std::list<HamProfile *> mPendingProfiles; // 0x38
    RecordScoreData mScoreData; // 0x40
    HamProfile *mUploadingProfile; // 0x54
    std::vector<LeaderboardRow> mRows; // 0x58
    std::map<unsigned int, std::vector<LeaderboardRow> > mCache; // 0x64
    int mMaxDownload; // 0x7c
    bool mReadingScores; // 0x80
    LeaderboardsType mType; // 0x84
    LeaderboardsMode mMode; // 0x88
    int mNumXuids; // 0x8c
    int mCurrentSongID; // 0x90
    bool mHideRank; // 0x94
    RCJob *mGetLeaderboardJob; // 0x98
    bool mEthernetDisconnected; // 0x9c
};

extern Leaderboards *TheLeaderboards;
