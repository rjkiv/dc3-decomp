#pragma once
#include "net_ham/ChallengeSystemJobs.h"

class ChallengeRecord {
public:
    ChallengeRecord(ChallengeRow);
    virtual ~ChallengeRecord() {}
    ChallengeRecord(const ChallengeRecord &);

    ChallengeRecord &operator=(const ChallengeRecord &other);

    const ChallengeRow &GetChallengeRow() const { return mData; }
    Symbol GetSong() const { return mSongShortName; }
    const char *GetSongTitle() const { return mSongTitle.Str(); }
    Symbol GetPlayerName() const { return mChallenger; }
    Symbol GetChallengeeName() const { return mChallengee; }
    int GetChallengeType() const { return mChallengeType; }

private:
    ChallengeRow mData; // 0x4
    Symbol mSongShortName; // 0x40
    Symbol mSongTitle; // 0x44
    Symbol mChallenger; // 0x48
    Symbol mChallengee; // 0x4c
    int mChallengeType; // 0x50 - not in pdb
};
