#pragma once
#include "NavListSortMgr.h"
#include "meta_ham/ChallengeRecord.h"
#include "net_ham/ChallengeSystemJobs.h"
#include "utl/Symbol.h"

class ChallengeSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SelectionIs(Symbol);
    virtual Symbol MoveOn(); // 0x78
    virtual void OnEnter();

    int GetTotalXpEarned(int);
    Symbol GetChallengerName();
    int GetChallengeExp(int);
    int GetPotentialChallengeExp(int);
    int GetSongID(int);
    Symbol GetSongShortName(int);
    String GetSongTitle(int);
    int GetOwnerChallengeScore(int);
    int GetOwnerChallengeTimeStamp(int);
    const char *GetBestchallengeScoreGamertag(int);
    int GetBestChallengeScore(int);
    int GetChallengerXp(int);
    int GetChallengeRecordSongType(int);
    const char *GetChallengerGamertag(int);
    int GetChallengeScore(int);

    static void Init(SongPreview &);
    static void Terminate();

private:
    ChallengeSortMgr(SongPreview &);
    ~ChallengeSortMgr();

protected:
    std::vector<ChallengeRecord> mChallengeRecords; // 0x78
};

extern ChallengeSortMgr *TheChallengeSortMgr;
