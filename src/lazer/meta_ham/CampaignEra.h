#pragma once

#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class CampaignEraSongEntry {
public:
    virtual ~CampaignEraSongEntry();

    CampaignEraSongEntry(DataArray *, DataArray *);
    bool HasCrazeMove(Symbol) const;
    Symbol GetCrazeMoveName(int) const;
    Symbol GetCrazeMoveVariantName(int) const;
    Symbol GetCrazeMoveHamMoveName(int) const;
    Symbol GetVariantFromHamMoveName(Symbol) const;
    Symbol GetHamMoveNameFromVariant(Symbol) const;

    Symbol m_symSong; // 0x4
    Symbol unk8;
    int unkc;
    std::vector<Symbol> m_vCrazeMoveNames; // 0x10
    std::vector<Symbol> m_vCrazeMoveVariantNames; // 0x1c
    std::vector<Symbol> m_vCrazeMoveHamMoveNames; // 0x28

private:
    void Configure(DataArray *, DataArray *);
};

class CampaignEra {
public:
    virtual ~CampaignEra();

    CampaignEra(DataArray *, DataArray *);
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

    Symbol mEra; // 0x4
    std::map<Symbol, int> unk8;
    std::vector<CampaignEraSongEntry *> m_vSongs; // 0x20
    Symbol mCrew; // 0x2c
    Symbol mVenue; // 0x30
    Symbol mEraSong_Unlocked_Token; // 0x34
    Symbol mEraSong_Complete_Token; // 0x38
    Symbol mEra_Intro_Movie; // 0x3c
    String unk40;
    String unk48;
    bool unk50;
    Symbol mCompletetion_Accomplishment; // 0x54
    int unk58;
    Symbol mCraze_Song; // 0x5c
    int unk60;
    Symbol mMastery_Stars[3]; // 0x64
    int unk70;
    int unk74;
    Symbol unk78;

private:
    void Cleanup();
    void Configure(DataArray *, DataArray *);
};
