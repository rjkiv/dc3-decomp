#pragma once
#include "net_ham/ChallengeSystemJobs.h"

class ChallengeRecord {
public:
    ChallengeRecord(ChallengeRow);
    virtual ~ChallengeRecord() {}
    ChallengeRecord(const ChallengeRecord &);

    ChallengeRecord &operator=(const ChallengeRecord &other);

    ChallengeRow &GetChallengeRow() { return mRow; }
    Symbol GetUnk40() { return unk40; }
    Symbol GetUnk44() { return unk44; }
    Symbol GetUnk48() { return unk48; }
    Symbol GetUnk4c() { return unk4c; }
    int GetUnk50() { return unk50; }

private:
    ChallengeRow mRow; // 0x4
    Symbol unk40; // 0x40
    Symbol unk44; // 0x44
    Symbol unk48; // 0x48
    Symbol unk4c; // 0x4c
    int unk50; // 0x50
};
