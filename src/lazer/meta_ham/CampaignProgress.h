#pragma once

#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/ContentMgr.h"
#include "stl/_map.h"
#include "utl/Loader.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class CampaignEraSongProgress : public FixedSizeSaveable {
public:
    virtual ~CampaignEraSongProgress();

    CampaignEraSongProgress();
    static int SaveSize(int);
    bool SetMoveMastered(Symbol, Symbol, Symbol);
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();

    int unk8;
    std::set<Symbol> unkc;
    bool unk24;
    bool unk25;
    int unk28;
    std::set<Symbol> unk2c;
    bool unk44;

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();
};

class CampaignEraProgress : public FixedSizeSaveable {
public:
    virtual ~CampaignEraProgress();

    CampaignEraProgress();
    CampaignEraProgress(Symbol);
    static int SaveSize(int);
    int GetTotalStarsEarned() const;
    int GetTotalMovesMastered() const;
    bool IsMoveMastered(Symbol, Symbol) const;
    bool IsMastered() const;
    bool IsEraComplete() const;
    bool IsPlayed() const;
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();
    void SetSongPlayed(Symbol, bool);
    void UpdateSong(Symbol, int);
    bool UpdateSongMoveMastered(Symbol, Symbol);
    CampaignEraSongProgress *GetEraSongProgress(Symbol) const;

    Symbol unk8;
    bool unkc;
    bool unkd;
    std::map<Symbol, CampaignEraSongProgress *> m_mapCampaignEraSongProgress; // 0x10

protected:
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Cleanup();
};

class CampaignProgress : public FixedSizeSaveable {
public:
    virtual ~CampaignProgress();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    CampaignProgress();
    void SetCampaignIntroCompleted(bool);
    void SetCampaignMindControlCompleted(bool);
    bool IsCampaignTanBattleCompleted() const;
    void SetCampaignTanBattleCompleted(bool);
    static int SaveSize(int);
    int GetRequiredStarsForDanceCrazeSong(Symbol) const;
    bool IsEraPlayed(Symbol) const;
    bool IsEraComplete(Symbol) const;
    bool IsEraMastered(Symbol) const;
    int GetEraStarsEarned(Symbol) const;
    int GetEraMovesMastered(Symbol) const;
    bool GetEraIntroMoviePlayed(Symbol) const;
    void SetEraIntroMoviePlayed(Symbol, bool);
    bool IsEraSongAvailable(Symbol, Symbol) const;
    bool IsSongPlayed(Symbol, Symbol) const;
    int GetSongStarsEarned(Symbol, Symbol) const;
    bool IsMoveMastered(Symbol, Symbol, Symbol) const;
    bool IsCampaignNew() const;
    Symbol GetFirstIncompleteEra() const;
    int GetStars() const;
    int GetNumCompletedEras() const;
    void Clear();
    bool IsDanceCrazeSongAvailable(Symbol) const;
    void ResetAllProgress();
    void ClearSongProgress(Symbol, Symbol);
    void BookmarkCurrentProgress();
    void ResetProgressToBookmark();
    void SetSongPlayed(Symbol, Symbol, bool);
    void UpdateEraSong(Symbol, Symbol, int);
    bool UpdateEraSongMoveMastered(Symbol, Symbol, Symbol);
    void UnlockAllMoves(Symbol, Symbol);

    int unk8;
    bool unkc;
    bool unkd;
    bool unke;
    std::map<Symbol, CampaignEraProgress *> unk10;
    int unk28;

private:
    CampaignEraProgress *GetEraProgress(Symbol) const;
};
