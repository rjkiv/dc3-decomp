#pragma once
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"

class GetLeaderboardByPlayerJob : public RCJob {
public:
    GetLeaderboardByPlayerJob(
        Hmx::Object *, HamProfile *, int, int, int, int, unsigned int
    );

    unsigned int unkb0;
};

class GetMiniLeaderboardJob : public RCJob {
public:
    GetMiniLeaderboardJob(Hmx::Object *, HamProfile const *, int);

    int unkb0;
};
