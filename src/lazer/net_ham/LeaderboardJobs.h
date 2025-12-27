#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"

class LeaderboardRow {
public:
    String unk0; // 0x0 - gamertag
    int unk8;
    int unkc; // 0xc - score
    unsigned int unk10; // 0x10 - rank fmt/rank/level?
    int unk14;
    Difficulty unk18; // 0x18 - difficulty
    bool unk1c; // 0x1c - no flashcards?
    bool unk1d;
    bool unk1e;
    XUID unk20;
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

private:
    unsigned int unkb0;
};

class GetMiniLeaderboardJob : public RCJob {
public:
    GetMiniLeaderboardJob(Hmx::Object *callback, const HamProfile *, int songID);
    void GetRows(std::vector<LeaderboardRow> *);

private:
    int mSongID; // 0xb0
};
