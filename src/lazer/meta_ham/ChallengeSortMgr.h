#pragma once
#include "NavListSortMgr.h"
#include "lazer/net_ham/ChallengeSystemJobs.h"

class ChallengeRecord {
public:
    ChallengeRecord(const ChallengeRecord &);
    virtual ~ChallengeRecord();

    ChallengeRecord &operator=(const ChallengeRecord &other);

    ChallengeRow &GetChallengeRow() { return mChallengeRow; }
    Symbol GetUnk40() { return unk40; }
    Symbol GetUnk44() { return unk44; }
    Symbol GetUnk48() { return unk48; }
    Symbol GetUnk4c() { return unk4c; }
    Symbol GetUnk50() { return unk50; }
protected:
    ChallengeRow mChallengeRow; // 0x4
    Symbol unk40; // 0x40
    Symbol unk44; // 0x44
    Symbol unk48; // 0x48
    Symbol unk4c; // 0x4c
    Symbol unk50; // 0x5c
};

class ChallengeSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual void OnEnter();
    virtual Symbol MoveOn();
    virtual bool SelectionIs();

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