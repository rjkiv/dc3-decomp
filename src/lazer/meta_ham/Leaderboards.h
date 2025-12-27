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

    std::list<SongStatusData> unk30;
    std::list<HamProfile *> mPendingProfiles; // 0x38
    RecordScoreData mRecordScoreData; // 0x40
    HamProfile *unk54;
    std::vector<LeaderboardRow> unk58;
    std::map<unsigned int, std::vector<LeaderboardRow> > unk64; // 0x64
    int unk7c;
    bool unk80;
    int unk84; // 0x84 - lb type
    int unk88; // 0x88 - lb mode
    int unk8c;
    int unk90; // 0x90 - song id?
    bool unk94;
    int unk98;
    bool unk9c;
};

extern Leaderboards *TheLeaderboards;
