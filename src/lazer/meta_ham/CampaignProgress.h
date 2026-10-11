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

    int GetStars() const { return m_iStars; }
    int GetMovesMastered() const { return m_MovesMastered.size(); }
    bool IsMoveMastered(Symbol move) const {
        return m_MovesMastered.find(move) != m_MovesMastered.end();
    }
    bool IsPlayed() const { return m_bPlayed; }
    void SetPlayed(bool played) { m_bPlayed = played; }
    void UpdateStars(int stars) { m_iStars = stars; }
    void ClearMovesMastered() { m_MovesMastered.clear(); }

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();

private:
    // current progress
    int m_iStars; // 0x8
    std::set<Symbol> m_MovesMastered; // 0xc
    bool m_bPlayed; // 0x24

    bool m_bReached; // 0x25

    // bookmarked progress
    int m_iStarsBookmark; // 0x28
    std::set<Symbol> m_MovesMasteredBookmark; // 0x2c
    bool m_bPlayedBookmark; // 0x44
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
    bool IsSongPlayed(Symbol song) const;
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();
    void SetSongPlayed(Symbol song, bool played);
    void UpdateSong(Symbol song, int stars);
    bool UpdateSongMoveMastered(Symbol song, Symbol move);
    CampaignEraSongProgress *GetEraSongProgress(Symbol song) const;

    static int SaveSize(int);

    bool GetIntroMoviePlayed() const { return m_bIntroMoviePlayed; }
    void SetIntroMoviePlayed(bool played) { m_bIntroMoviePlayed = played; }

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();

private:
    Symbol m_symName; // 0x8
    bool m_bIntroMoviePlayed; // 0xc
    bool m_bIntroSequencePlayed; // 0xd
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

    void SetParent(HamProfile *p) { m_pParentProfile = p; }
    int GetMasterQuestSongCountForRequiredStars() const { return m_masterQuestSongs; }
    void IncrementMasterQuestSongCountForRequiredStars() { m_masterQuestSongs++; }

private:
    CampaignEraProgress *GetEraProgress(Symbol era) const;

    HamProfile *m_pParentProfile; // 0x8
    bool m_bCampaignIntroCompleted; // 0xc
    bool m_bCampaignMindControlCompleted; // 0xd - not in pdb
    bool m_bCampaignTanBattleCompleted; // 0xe
    // key = era, value = campaign era progress
    std::map<Symbol, CampaignEraProgress *> m_mapCampaignEraProgress; // 0x10
    int m_masterQuestSongs; // 0x28
};
