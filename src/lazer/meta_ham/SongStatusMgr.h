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
        mPercentPassed = 0;
        mNumPerfects = 0;
        mNumNices = 0;
        unk10 = 0;
        mNoFlashcards = 0;
        mNeedUpload = false;
    }

    int mScore; // 0x0
    int mPracticeScore; // 0x4
    int mCoopScore; // 0x8
    unsigned char mStars; // 0xc
    unsigned char mPercentPassed; // 0xd
    unsigned char mNumPerfects; // 0xe
    unsigned char mNumNices; // 0xf
    bool unk10;
    bool mNoFlashcards; // 0x11
    bool mNeedUpload; // 0x12
    Difficulty mDifficulty; // 0x14
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
    SongStatusData mStatusData[kNumDifficulties]; // 0x4
    unsigned int mLastPlayed; // 0x74
    unsigned char unk78; // 0x78 - stars related
    bool unk79; // 0x79
    Difficulty unk7c; // 0x7c
    int mLastScore; // 0x80
    int unk84;
    int mBattleScore; // 0x88
    int mTotalBattleWins; // 0x8c
    int mTotalBattleLosses; // 0x90
    bool mWonLastBattle; // 0x94
    unsigned int mLastPlayedPractice; // 0x98
    int unk9c;
    int unka0;
    int unka4;
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

    void GetScoresToUpload(std::list<SongStatusData> &);
    void GetFlauntsToUpload(std::list<FlauntStatusData> &);
    bool HasSongStatus(int) const;
    const SongStatus &GetSongStatus(int) const;
    Difficulty GetDifficulty(int) const;
    int GetScore(int, bool &) const;
    bool IsSongPlayed(int) const;
    int GetCoopScore(int) const;
    int GetScoreForDifficulty(int, Difficulty, bool &) const;
    int GetBestScore(int, bool &, Difficulty) const;
    int GetStars(int, bool &) const;
    int GetStarsForDifficulty(int, Difficulty, bool &) const;
    int GetBestStars(int, bool &, Difficulty) const;
    int GetPercentForDifficulty(int, Difficulty) const;
    int GetNumPerfectForDifficulty(int, Difficulty) const;
    int GetNumNiceForDifficulty(int, Difficulty) const;
    int GetBestBattleScore(int) const;
    int GetTotalBattleWins(int) const;
    int GetTotalBattleLosses(int) const;
    bool GetLastBattleResult(int) const;
    unsigned int GetLastPlayed(int) const;
    int GetLastScore(int, bool &) const;
    unsigned int GetLastPlayedPractice(int) const;
    int GetPracticeScore(int) const;
    int GetPracticeScore(int, Difficulty) const;
    Difficulty GetPracticeDifficulty(int) const;
    void Clear();
    void ClearNeedUpload(int, Difficulty);
    void ClearFlauntsNeedUpload(int);
    int CalculateTotalScore(Symbol) const;
    bool UpdateSong(int, int, int, Difficulty, int, int, int, int, int, bool, bool, bool);
    bool UpdateBattleSong(int, int, bool);
    bool UpdateFlaunt(int, int, Difficulty, bool);

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

    SongStatus &AccessSongStatus(int);

    HamSongMgr *mSongMgr; // 0x38
    // key = song ID, value = song status
    std::map<int, SongStatus> mSongStatusMap; // 0x3c
};
