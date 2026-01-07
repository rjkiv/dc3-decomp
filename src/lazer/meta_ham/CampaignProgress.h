#pragma once
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/ContentMgr.h"
#include "utl/Loader.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <map>

class HamProfile;

class CampaignEraSongProgress : public FixedSizeSaveable {
public:
    enum {
        kMaxSymbols_CampaignMovesPerSong = 4
    };
    CampaignEraSongProgress();
    virtual ~CampaignEraSongProgress();

    bool SetMoveMastered(Symbol era, Symbol song, Symbol move);
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();

    static int SaveSize(int);

    int GetStarsEarned() const { return mStarsEarned; }
    int NumMovesMastered() const { return m_MovesMastered.size(); }
    bool IsMoveMastered(Symbol move) const {
        return m_MovesMastered.find(move) != m_MovesMastered.end();
    }
    bool IsSongPlayed() const { return mSongPlayed; }
    void SetSongPlayed(bool played) { mSongPlayed = played; }
    void SetStarsEarned(int stars) { mStarsEarned = stars; }
    void ClearMovesMastered() { m_MovesMastered.clear(); }

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();

    // current progress
    int mStarsEarned; // 0x8
    std::set<Symbol> m_MovesMastered; // 0xc
    bool mSongPlayed; // 0x24

    bool unk25; // 0x25

    // bookmarked progress
    int mBookmarkedStarsEarned; // 0x28
    std::set<Symbol> mBookmarkedMovesMastered; // 0x2c
    bool mBookmarkedSongPlayed; // 0x44
};

class CampaignEraProgress : public FixedSizeSaveable {
public:
    enum {
        kMaxCampaignEraSongs = 10
    };
    CampaignEraProgress();
    CampaignEraProgress(Symbol era);
    virtual ~CampaignEraProgress();

    int GetTotalStarsEarned() const;
    int GetTotalMovesMastered() const;
    bool IsMoveMastered(Symbol song, Symbol move) const;
    bool IsMastered() const;
    bool IsEraComplete() const;
    bool IsPlayed() const;
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();
    void SetSongPlayed(Symbol song, bool played);
    void UpdateSong(Symbol song, int stars);
    bool UpdateSongMoveMastered(Symbol song, Symbol move);
    CampaignEraSongProgress *GetEraSongProgress(Symbol song) const;

    static int SaveSize(int);

    bool GetIntroMoviePlayed() const { return mIntroMoviePlayed; }
    void SetIntroMoviePlayed(bool played) { mIntroMoviePlayed = played; }

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();

    Symbol mEra; // 0x8
    bool mIntroMoviePlayed; // 0xc
    bool unkd; // 0xd
    // key = song shortname, value = campaign era song progress
    std::map<Symbol, CampaignEraSongProgress *> m_mapCampaignEraSongProgress; // 0x10
};

class CampaignProgress : public FixedSizeSaveable {
public:
    enum {
        kMaxSymbols_CampaignEras = 10
    };
    CampaignProgress();
    virtual ~CampaignProgress();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void SetCampaignIntroCompleted(bool complete);
    void SetCampaignMindControlCompleted(bool complete);
    bool IsCampaignTanBattleCompleted() const;
    void SetCampaignTanBattleCompleted(bool complete);
    static int SaveSize(int);
    int GetRequiredStarsForDanceCrazeSong(Symbol era) const;
    bool IsEraPlayed(Symbol era) const;
    bool IsEraComplete(Symbol era) const;
    bool IsEraMastered(Symbol era) const;
    int GetEraStarsEarned(Symbol era) const;
    int GetEraMovesMastered(Symbol era) const;
    bool GetEraIntroMoviePlayed(Symbol era) const;
    void SetEraIntroMoviePlayed(Symbol era, bool played);
    bool IsEraSongAvailable(Symbol era, Symbol song) const;
    bool IsSongPlayed(Symbol era, Symbol song) const;
    int GetSongStarsEarned(Symbol era, Symbol song) const;
    bool IsMoveMastered(Symbol era, Symbol song, Symbol move) const;
    bool IsCampaignNew() const;
    bool IsCampaignIntroCompleted() const;
    bool IsCampaignMindControlCompleted() const;
    Symbol GetFirstIncompleteEra() const;
    int GetStars() const;
    int GetNumCompletedEras() const;
    void Clear();
    bool IsDanceCrazeSongAvailable(Symbol era) const;
    void ResetAllProgress();
    void ClearSongProgress(Symbol era, Symbol song);
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();
    void SetSongPlayed(Symbol era, Symbol song, bool played);
    void UpdateEraSong(Symbol era, Symbol song, int stars);
    bool UpdateEraSongMoveMastered(Symbol era, Symbol song, Symbol move);
    void UnlockAllMoves(Symbol era, Symbol song);

    void SetProfile(HamProfile *p) { mProfile = p; }
    int Num5StarredMQSongs() const { return mNum5StarredMQSongs; }

private:
    CampaignEraProgress *GetEraProgress(Symbol era) const;

    HamProfile *mProfile; // 0x8
    bool mIntroCompleted; // 0xc
    bool mMindControlCompleted; // 0xd
    bool mTanBattleCompleted; // 0xe
    // key = era, value = campaign era progress
    std::map<Symbol, CampaignEraProgress *> m_mapCampaignEraProgress; // 0x10
    int mNum5StarredMQSongs; // 0x28 - num master quest songs you earned 5 stars on
};
