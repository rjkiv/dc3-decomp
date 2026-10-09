#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"

class LeaderboardRow {
public:
    String gamertag; // 0x0
    unsigned int pid; // 0x8
    unsigned int score; // 0xc
    unsigned int rank; // 0x10
    unsigned int orank; // 0x14
    Difficulty diff; // 0x18
    bool noFlashcards; // 0x1c
    bool isPercentile; // 0x1d
    bool isFriend; // 0x1e
    XUID xuid; // 0x20
};

class GetLeaderboardByPlayerJob : public RCJob {
public:
    GetLeaderboardByPlayerJob(
        Hmx::Object *callback,
        HamProfile *,
        int songID,
        int typeID,
        int modeID,
        int numRows,
        unsigned int
    );
    void GetRows(std::vector<LeaderboardRow> *);
    unsigned int GetChecksum() const { return mChecksum; }

private:
    unsigned int mChecksum; // 0xa0 on pdb but 0xb0 here ?
};

class GetMiniLeaderboardJob : public RCJob {
public:
    GetMiniLeaderboardJob(Hmx::Object *callback, const HamProfile *, int songID);
    void GetRows(std::vector<LeaderboardRow> *);

    int mSongID; // 0xb0
};
