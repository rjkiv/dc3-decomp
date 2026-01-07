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

    Symbol GetSongName() const { return m_symSong; }
    Symbol GetUnk8() const { return unk8; }
    int GetSongRequiredStars() const { return m_iRequiredStars; }
    int GetNumSongCrazeMoves() const { return m_vCrazeMoveNames.size(); }

private:
    void Configure(DataArray *, DataArray *);

    Symbol m_symSong; // 0x4
    Symbol unk8; // 0x8
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

    int GetNumSongs() { return m_vSongs.size(); }
    CampaignEraSongEntry *SongEntryAtIndex(int idx) { return m_vSongs[idx]; }
    Symbol Crew() const { return mCrew; }
    Symbol Venue() const { return mVenue; }
    int StarsRequiredForOutfits() const { return mStarsRequiredForOutfits; }
    int StarsRequiredForMastery() const { return mStarsRequiredForMastery; }
    Symbol CompletionAccomplishment() const { return mCompletionAccomplishment; }
    Symbol EraSongUnlockedToken() const { return mEraSongUnlockedToken; }
    Symbol EraSongCompleteToken() const { return mEraSongCompleteToken; }
    int MovesRequiredForMastery() const { return mMovesRequiredForMastery; }
    Symbol OutfitAward() const { return mOutfitAward; }
    Symbol GetMasteryStars(Difficulty d) { return mMasteryStars[d]; }
    bool GetUnk50() const { return unk50; }

protected:
    Symbol mEra; // 0x4
    std::map<Symbol, int> unk8; // 0x8
    std::vector<CampaignEraSongEntry *> m_vSongs; // 0x20
    Symbol mCrew; // 0x2c
    Symbol mVenue; // 0x30
    Symbol mEraSongUnlockedToken; // 0x34
    Symbol mEraSongCompleteToken; // 0x38
    Symbol mEraIntroMovie; // 0x3c
    String unk40;
    String unk48;
    bool unk50;
    Symbol mCompletionAccomplishment; // 0x54
    int unk58;
    Symbol mCrazeSong; // 0x5c
    int mStarsRequiredForMastery; // 0x60
    Symbol mMasteryStars[3]; // 0x64
    int mMovesRequiredForMastery; // 0x70
    int mStarsRequiredForOutfits; // 0x74
    Symbol mOutfitAward; // 0x78

private:
    void Cleanup();
    void Configure(DataArray *, DataArray *);
};
