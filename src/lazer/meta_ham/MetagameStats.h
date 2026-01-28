#pragma once
#include "hamobj/HamPlayerData.h"
#include "meta/FixedSizeSaveable.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"

class HamProfile;

enum StatLayout {
    kStatsLayoutHeader = -1,
    kStatsLayoutSpacer = -2,
};

enum StatType {
    kStatType_Count = 0,
    kStatType_Time = 1,
    kStatType_FavoriteMode = 2,
    kStatType_FavoriteSong = 3,
    kStatType_FavoriteCharacter = 4
};

class MetagameStats : public Hmx::Object,
                      public UIListProvider,
                      public FixedSizeSaveable {
public:
    struct FavoriteStat {
        // key:
        //  -song id for favorite song metrics
        //  -index in big char array macro for character metric
        //  -mode enum for favorite mode
        // value = the relevant count
        std::map<int, int> mCounts;
    };
    enum CountStatID {
        kCountStat_StatScreenVisits = 0,
        kCountStat_TotalTimePlayed = 1,
        kCountStat_TotalTimePerforming = 2,
        kCountStat_TotalTimeRehearsing = 3,
        kCountStat_TotalTimeMultiplayer = 4,
        kCountStat_TimesPlayedPerform = 5,
        kCountStat_TimesPlayedPractice = 6,
        kCountStat_TimesPlayedMultiplayer = 7,
        kCountStat_StarsEarned = 8,
        kCountStat_StarsEarnedPerform = 9,
        kCountStat_StarsEarnedMultiplayer = 10,
        kCountStat_TotalFitnessSongs = 11,
        kCountStat_TotalTimeFitness = 12,
        kCountStat_TotalCaloriesBurned = 13,
        kCountStat_PhotosTaken = 14,
        kCountStat_PhotosTakenPerform = 15,
        kCountStat_PhotosTakenMultiplayer = 16,
        kCountStat_InifinitePlaylistScore = 17,
        kCountStat_InifinitePlaylistTime = 18,
        kCountStat_LongestPartyAttended = 19,
        kCountStat_MostConsecutiveDaysPlayed = 20,
        kCountStat_TimesChallengesSent = 21,
        kCountStat_TimesChallengesMet = 22,
        kNumCountStats = 23,
    };
    enum FavoriteStatID {
        kFavoriteStat_FavoriteMode = 0,
        kFavoriteStat_FavoriteSong = 1,
        kFavoriteStat_FavoriteCharacter = 2,
        kFavoriteStat_FavoriteSongPerform = 3,
        kFavoriteStat_FavoriteSongPractice = 4,
        kFavoriteStat_FavoriteSongMultiplayer = 5,
        kFavoriteStat_FavoriteSongFitness = 6,
        kNumFavoriteStats = 7,
    };

    MetagameStats();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual int NumData() const { return mStatsCfg->Size() - 1; }

    void Clear();
    void IncrementCount(CountStatID, int);
    void TallyFavorite(FavoriteStatID, int, int);
    int GetCount(CountStatID) const;
    int GetFavorite(FavoriteStatID) const;
    int GetFavoriteCount(FavoriteStatID, int) const;
    void UpdatePartyStats(int);
    void PhotoTaken();
    void WriteTimePlayed(HamProfile *, int);
    bool InqStatString(int, String &);
    void HandleGameplayEnded(HamProfile *, HamPlayerData *, const EndGameResult &);

    __forceinline void IncFavoriteStat(FavoriteStatID id, int key) {
        mFavoriteStats[id].mCounts[key]++;
        mDirty = true;
    }
    void AddCount(CountStatID id, int count) {
        mCountStats[id] += count;
        mDirty = true;
    }
    void UpdateInfinitePlaylistScore(int score) {
        if (score > mCountStats[kCountStat_InifinitePlaylistScore]) {
            mCountStats[kCountStat_InifinitePlaylistScore] = score;
            mDirty = true;
        }
    }
    void UpdateInfinitePlaylistTime(int time) {
        if (time > mCountStats[kCountStat_InifinitePlaylistTime]) {
            mCountStats[kCountStat_InifinitePlaylistTime] = time;
            mDirty = true;
        }
    }

    static int SaveSize(int);

    bool IsDirty() const { return mDirty; }

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    Symbol SetTitleByThreshold(int, DataArray *) const;
    Symbol SetTitleForFavorite(int, int, DataArray *) const;

    int mCountStats[kNumCountStats]; // 0x38
    FavoriteStat mFavoriteStats[kNumFavoriteStats]; // 0x94
    DataArray *mStatsCfg; // 0x13c
    int unk140; // 0x140 - total time played?
    bool mDirty; // 0x144
};
