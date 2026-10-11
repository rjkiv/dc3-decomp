#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamSongMgr.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Data.h"
#include "os/ContentMgr.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

// size 0x1c
struct SongStatusData {
    void SaveToStream(BinStream &) const;
    void LoadFromStream(BinStream &);

    void Clear() {
        mScore = 0;
        mPracticeScore = 0;
        mCoopScore = 0;
        mStars = 0;
        mMovesPercent = 0;
        mNumPerfect = 0;
        mNumNice = 0;
        mStarsNoFlashcards = false;
        mScoreNoFlashcards = false;
        mNeedUpload = false;
    }

    int mScore; // 0x0
    int mPracticeScore; // 0x4
    int mCoopScore; // 0x8
    unsigned char mStars; // 0xc
    unsigned char mMovesPercent; // 0xd
    unsigned char mNumPerfect; // 0xe
    unsigned char mNumNice; // 0xf
    bool mStarsNoFlashcards; // 0x10
    bool mScoreNoFlashcards; // 0x11
    bool mNeedUpload; // 0x12
    Difficulty mDiff; // 0x14
    int mSongID; // 0x18
};

struct FlauntStatusData {
    void SaveToStream(BinStream &) const;
    void LoadFromStream(BinStream &);

    int mScore; // 0x0
    Difficulty mDiff; // 0x4
    bool mNeedUpload; // 0x8
    int mSongID; // 0xc
};

class SongStatus {
public:
    SongStatus();
    SongStatus(int songID);
    void Clear();
    const SongStatusData &GetBestSongStatusData() const;
    const SongStatusData &GetBestPracticeSongStatusData() const;

    int mSongID; // 0x0
    SongStatusData mSongData[kNumDifficulties]; // 0x4
    unsigned int mLastPlayed; // 0x74
    unsigned char mLastStars; // 0x78
    bool mLastStarsNoFlashcards; // 0x79
    Difficulty mLastDiff; // 0x7c
    int mLastScore; // 0x80
    int mLastCoopScore; // 0x84
    int mBattleScore; // 0x88
    int mBattleWins; // 0x8c
    int mBattleLosses; // 0x90
    bool mWonLastDanceBattle; // 0x94
    unsigned int mLastPlayedPractice; // 0x98
    int mLastScorePractice; // 0x9c
    Difficulty mLastDiffPractice; // 0xa0
    int mFlags; // 0xa4
    FlauntStatusData mFlauntData; // 0xa8
};

BinStream &operator<<(BinStream &, const SongStatus &);
BinStream &operator>>(BinStream &, SongStatus &);

class SongStatusMgr : public Hmx::Object,
                      public FixedSizeSaveable,
                      public ContentMgr::Callback {
public:
    SongStatusMgr(HamSongMgr *);
    // Hmx::Object
    virtual ~SongStatusMgr() {}
    virtual DataNode Handle(DataArray *, bool);

    void GetScoresToUpload(std::list<SongStatusData> &songStatusData);
    void GetFlauntsToUpload(std::list<FlauntStatusData> &flauntStatusData);
    bool HasSongStatus(int songID) const;
    const SongStatus &GetSongStatus(int songID) const;
    Difficulty GetDifficulty(int songID) const;
    int GetScore(int songID, bool &noFlashcards) const;
    bool IsSongPlayed(int songID) const;
    int GetCoopScore(int) const;
    int GetScoreForDifficulty(int songID, Difficulty diff, bool &noFlashcards) const;
    int GetBestScore(int songID, bool &noFlashcards, Difficulty diff) const;
    int GetStars(int songID, bool &) const;
    int GetStarsForDifficulty(int songID, Difficulty diff, bool &noFlashcards) const;
    int GetBestStars(int songID, bool &, Difficulty diff) const;
    int GetPercentForDifficulty(int songID, Difficulty diff) const;
    int GetNumPerfectForDifficulty(int songID, Difficulty diff) const;
    int GetNumNiceForDifficulty(int songID, Difficulty diff) const;
    int GetBestBattleScore(int songID) const;
    int GetTotalBattleWins(int songID) const;
    int GetTotalBattleLosses(int songID) const;
    bool GetLastBattleResult(int songID) const;
    unsigned int GetLastPlayed(int songID) const;
    int GetLastScore(int songID, bool &) const;
    unsigned int GetLastPlayedPractice(int songID) const;
    int GetPracticeScore(int songID) const;
    int GetPracticeScore(int songID, Difficulty diff) const;
    Difficulty GetPracticeDifficulty(int songID) const;
    void Clear();
    void ClearNeedUpload(int songID, Difficulty diff);
    void ClearFlauntsNeedUpload(int songID);
    int CalculateTotalScore(Symbol game_origin) const;
    bool UpdateSong(
        int songID, int score, int, Difficulty diff, int, int, int, int, int, bool, bool, bool
    );
    bool UpdateBattleSong(int songID, int score, bool won);
    bool UpdateFlaunt(int songID, int score, Difficulty diff, bool);

    static int SaveSize(int);
    static void Init();
    static bool sFakeLeaderboardUploadFailure;

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &fs) const {
        SaveStd(fs, mSongStatusMap, 0xD48, 0x83);
    }
    virtual void LoadFixed(FixedSizeSaveableStream &fs, int) {
        LoadStd(fs, mSongStatusMap, 0xD48, 0x83);
    }

    SongStatus &AccessSongStatus(int songID);

    HamSongMgr *mSongMgr; // 0x38
    // key = song ID, value = song status
    std::map<int, SongStatus> mSongStatusMap; // 0x3c
};
