#pragma once
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"

// size 0x3c
class ChallengeRow {
public:
    enum Type {
        kChallengeHmxGold = 0,
        kChallengeHmxSilver = 1,
        kChallengeHmxBronze = 2,
        kChallengeDlcGold = 3,
        kChallengeDlcSilver = 4,
        kChallengeDlcBronze = 5,
        kNumChallengeTypes = 6
    };
    bool operator!=(const ChallengeRow &other) const {
        return (unsigned int)unk0 != other.unk0 || mGamertag != other.mGamertag
            || (unsigned int)mSongID != other.mSongID || mArtist != other.mArtist
            || mSongTitle != other.mSongTitle || (unsigned int)mScore != other.mScore
            || (unsigned int)mDiff != other.mDiff || (unsigned int)mType != other.mType
            || unk2c != other.unk2c || mTimeStamp != other.mTimeStamp
            || (unsigned int)mChallengerXp != other.mChallengerXp;
    }
    bool IsHMXChallenge() const {
        return mType >= kChallengeHmxGold && mType <= kChallengeHmxBronze;
    }
    bool IsDLCChallenge() const {
        return mType >= kChallengeDlcGold && mType <= kChallengeDlcBronze;
    }

    int unk0; // 0x0
    String mGamertag; // 0x4
    int mSongID; // 0xc
    String mArtist; // 0x10
    String mSongTitle; // 0x18
    int mScore; // 0x20
    // difficulty/"dots" of this song, 0-7?
    // doesn't look like the choreo difficulty easy/medium/expert
    int mDiff; // 0x24
    Type mType; // 0x28
    String unk2c; // 0x2c
    unsigned int mTimeStamp; // 0x34
    int mChallengerXp; // 0x38
};

enum ChallengeBadgeType {
    kBadgeGold = 0,
    kBadgeSilver = 1,
    kBadgeBronze = 2,
    kNumBadgeTypes = 3
};

class ChallengeBadgeInfo {
public:
    int mMedalCounts[kNumBadgeTypes]; // 0x0
    // int mGold; // 0x0
    // int mSilver; // 0x4
    // int mBronze; // 0x8
};

class FlauntScoreData {
public:
    FlauntScoreData() : mProfile(0), mStatus(0) {}
    virtual ~FlauntScoreData() {}

    HamProfile *mProfile; // 0x4
    FlauntStatusData *mStatus; // 0x8
};

class FlauntScoreJob : public RCJob {
public:
    FlauntScoreJob(Hmx::Object *callback, FlauntScoreData &data);
};

class GetPlayerChallengesJob : public RCJob {
public:
    GetPlayerChallengesJob(Hmx::Object *callback, std::vector<HamProfile *> &profiles);
    void GetRows(std::map<String, std::vector<ChallengeRow> > &, bool &);
};

class GetOfficialChallengesJob : public RCJob {
public:
    GetOfficialChallengesJob(Hmx::Object *callback);
    void GetRows(std::vector<ChallengeRow> &, double &, bool &);
};

class GetChallengeBadgeCountsJob : public RCJob {
public:
    GetChallengeBadgeCountsJob(Hmx::Object *callback, std::vector<HamProfile *> &profiles);
    void GetBadgeInfo(std::map<String, ChallengeBadgeInfo> &);
};
