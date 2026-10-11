#pragma once
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <vector>

class CampaignEraSongEntry {
public:
    CampaignEraSongEntry(DataArray *, DataArray *);
    virtual ~CampaignEraSongEntry();

    bool HasCrazeMove(Symbol) const;
    Symbol GetCrazeMoveName(int) const;
    Symbol GetCrazeMoveVariantName(int) const;
    Symbol GetCrazeMoveHamMoveName(int) const;
    Symbol GetVariantFromHamMoveName(Symbol) const;
    Symbol GetHamMoveNameFromVariant(Symbol) const;

    Symbol GetSong() const { return m_symSong; }
    Symbol GetCharacterForSong() const { return m_symCharacter; }
    int GetRequiredStars() const { return m_iRequiredStars; }
    int GetNumCrazeMoves() const { return m_vCrazeMoveNames.size(); }

private:
    void Configure(DataArray *, DataArray *);

    Symbol m_symSong; // 0x4
    Symbol m_symCharacter; // 0x8
    int m_iRequiredStars; // 0xc
    std::vector<Symbol> m_vCrazeMoveNames; // 0x10
    std::vector<Symbol> m_vCrazeMoveVariantNames; // 0x1c
    std::vector<Symbol> m_vCrazeMoveHamMoveNames; // 0x28
};

class CampaignEra {
public:
    CampaignEra(DataArray *, DataArray *);
    virtual ~CampaignEra();

    Symbol GetDanceCrazeSong() const;
    bool IsTanBattleEra() const;
    int GetMaxStars() const;
    CampaignEraSongEntry *GetSongEntry(int) const;
    CampaignEraSongEntry *GetSongEntry(Symbol) const;
    Symbol GetSongName(int) const;
    int GetSongIndex(Symbol) const;
    int GetNumSongCrazeMoves(Symbol) const;
    int GetSongRequiredStars(Symbol);
    bool HasCrazeMove(Symbol, Symbol) const;
    Symbol GetMoveVariantName(Symbol, Symbol) const;
    Symbol GetHamMoveNameFromVariant(Symbol, Symbol) const;
    Symbol GetIntroMovie() const;
    Symbol GetName() const;

    int GetNumSongs() const { return m_vSongs.size(); }
    CampaignEraSongEntry *SongEntryAtIndex(int idx) { return m_vSongs[idx]; } // hmm
    Symbol GetCrew() const { return m_symEraCrew; }
    Symbol GetVenue() const { return m_symEraVenue; }
    int GetRequiredStarsForOutfits() const { return m_iOutfitStars; }
    int GetRequiredStarsForMastery() const { return m_iMasteryStars; }
    Symbol GetCompletionAccomplishment() const { return m_symCompletionAccomplishment; }
    Symbol GetSongUnlockedToken() const { return m_symSongUnlocked; }
    Symbol GetCompleteToken() const { return m_symComplete; }
    int GetRequiredMovesForMastery() const { return m_iMasteryMoves; }
    Symbol GetOutfitAward() const { return m_symOutfitAward; }
    Symbol GetMasteryGoal(Difficulty d) { return m_symMasteryGoal[d]; }
    bool IsFinalTimeTravelEra() const { return m_bIsFinalTimeTravelEra; }
    void SetFinalTimeTravelEra(bool b) { m_bIsFinalTimeTravelEra = b; }
    const std::vector<CampaignEraSongEntry *> &Songs() const { return m_vSongs; }

private:
    void Cleanup();
    void Configure(DataArray *, DataArray *);

    Symbol m_symName; // 0x4
    std::map<Symbol, int> m_mapSongLookup; // 0x8
    std::vector<CampaignEraSongEntry *> m_vSongs; // 0x20
    Symbol m_symEraCrew; // 0x2c
    Symbol m_symEraVenue; // 0x30
    Symbol m_symSongUnlocked; // 0x34
    Symbol m_symComplete; // 0x38
    Symbol m_symIntroMovie; // 0x3c
    String m_strHighlightSound; // 0x40
    String m_strSelectSound; // 0x48
    bool m_bIsFinalTimeTravelEra; // 0x50
    Symbol m_symCompletionAccomplishment; // 0x54
    int m_iEraSongStars; // 0x58
    Symbol m_symDanceCrazeSong; // 0x5c
    int m_iMasteryStars; // 0x60
    Symbol m_symMasteryGoal[3]; // 0x64
    int m_iMasteryMoves; // 0x70
    int m_iOutfitStars; // 0x74
    Symbol m_symOutfitAward; // 0x78
};
