#pragma once
#include "hamobj/HamPlayerData.h"
#include "meta_ham/Accomplishment.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class AccomplishmentCharacterListConditional : public AccomplishmentConditional {
public:
    AccomplishmentCharacterListConditional(DataArray *, int);
    virtual ~AccomplishmentCharacterListConditional();
    virtual AccomplishmentType GetType() const {
        return kAccomplishmentTypeCharacterListConditional;
    }
    virtual bool IsRelevantForSong(Symbol) const { return true; }

    bool AreOldOutfitListConditionsMet();
    bool AreCharacterListConditionsMet(
        Symbol shortname, HamPlayerData *hpd, HamProfile *profile
    );
    bool AreUnlockableOutfitListConditionsMet(HamPlayerData *hpd, HamProfile *profile);

private:
    void Configure(DataArray *);

    std::vector<Symbol> mChars; // 0x70
    std::vector<Symbol> mCrewChars; // 0x7c
    std::vector<Symbol> mOldOutfits; // 0x88
    std::vector<Symbol> mUnlockableOutfits; // 0x94
    std::vector<bool> unka0; // 0xa0 - unlockable outfits unlocked?
};
