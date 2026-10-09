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
        return id != other.id || challengerUsername != other.challengerUsername
            || songId != other.songId || artistName != other.artistName
            || songName != other.songName || score != other.score || diff != other.diff
            || (unsigned int)type != other.type
            || challengeeUsername != other.challengeeUsername
            || dateTime != other.dateTime || xp != other.xp;
    }
    bool IsHMXChallenge() const {
        return type >= kChallengeHmxGold && type <= kChallengeHmxBronze;
    }
    bool IsDLCChallenge() const {
        return type >= kChallengeDlcGold && type <= kChallengeDlcBronze;
    }

    unsigned int id; // 0x0
    String challengerUsername; // 0x4
    unsigned int songId; // 0xc
    String artistName; // 0x10
    String songName; // 0x18
    unsigned int score; // 0x20
    // difficulty/"dots" of this song, 0-7?
    // doesn't look like the choreo difficulty easy/medium/expert
    unsigned int diff; // 0x24
    Type type; // 0x28
    String challengeeUsername; // 0x2c
    unsigned int dateTime; // 0x34
    unsigned int xp; // 0x38
};

enum ChallengeBadgeType {
    kGold = 0x0000,
    kSilver = 0x0001,
    kBronze = 0x0002,
    kNumBadgeTypes = 0x0003,
};

class ChallengeBadgeInfo {
public:
    ChallengeBadgeInfo() {
        for (int i = 0; i < kNumBadgeTypes; i++) {
            badge[i] = 0;
        }
    }

    int badge[kNumBadgeTypes]; // 0x0
    // int mGold; // 0x0
    // int mSilver; // 0x4
    // int mBronze; // 0x8
};

class FlauntScoreData {
public:
    FlauntScoreData() : mProfile(0), mFlauntData(0) {}
    virtual ~FlauntScoreData() {}

    HamProfile *mProfile; // 0x4
    FlauntStatusData *mFlauntData; // 0x8
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
