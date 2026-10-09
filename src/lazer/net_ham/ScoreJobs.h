#pragma once
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"

class RecordScoreData {
public:
    RecordScoreData()
        : mProfile(0), mSongData(0), mCareerScore(0), mCappedCareerScore(0) {}
    virtual ~RecordScoreData() {}

    const HamProfile *mProfile; // 0x4
    const SongStatusData *mSongData; // 0x8
    unsigned int mCareerScore; // 0xc - c score
    unsigned int mCappedCareerScore; // 0x10 - cc score
};

class RecordScoreJob : public RCJob {
public:
    RecordScoreJob(
        Hmx::Object *callback, RecordScoreData &data, int songID, bool provideInstarank
    );
};
