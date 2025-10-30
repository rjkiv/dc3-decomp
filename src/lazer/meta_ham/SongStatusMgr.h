#pragma once

#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/HamSongMgr.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/ContentMgr.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

struct SongStatusData {
    void SaveToStream(BinStream &) const;
    void LoadFromStream(BinStream &);
};

struct FlauntStatusData {
    void SaveToStream(BinStream &) const;
    void LoadFromStream(BinStream &);
};

class SongStatus {
public:
    SongStatus();
    SongStatus(int);
    void Clear();
    SongStatusData const &GetBestSongStatusData() const;
    SongStatusData const &GetBestPracticeSongStatusData() const;
};

class SongStatusMgr : public Hmx::Object, public FixedSizeSaveable {
public:
    virtual ~SongStatusMgr();

    SongStatusMgr(HamSongMgr *);
    static int SaveSize(int);
    static void Init();
    void GetScoresToUpload(std::list<SongStatusData> &);
    void GetFlauntsToUpload(std::list<FlauntStatusData> &);
    bool HasSongStatus(int) const;
    SongStatus const &GetSongStatus(int) const;
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

    ContentMgr::Callback *unk34;
    HamSongMgr *unk38;
    std::map<int, SongStatus> unk3c;

private:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    SongStatus &AccessSongStatus(int);
};
