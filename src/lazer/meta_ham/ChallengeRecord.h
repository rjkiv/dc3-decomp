#pragma once
#include "net_ham/ChallengeSystemJobs.h"

class ChallengeRecord {
public:
    ChallengeRecord(ChallengeRow);
    virtual ~ChallengeRecord() {}

private:
    ChallengeRow mRow; // 0x4
    Symbol unk40; // 0x40
    Symbol unk44; // 0x44
    Symbol unk48; // 0x48
    Symbol unk4c; // 0x4c
    int unk50; // 0x50
};
