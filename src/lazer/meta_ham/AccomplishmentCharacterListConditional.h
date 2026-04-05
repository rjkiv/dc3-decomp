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
    bool AreCharacterListConditionsMet(Symbol, HamPlayerData *, HamProfile *);
    bool AreUnlockableOutfitListConditionsMet(HamPlayerData *, HamProfile *);

private:
    void Configure(DataArray *);

    std::vector<Symbol> unk70;
    std::vector<Symbol> unk7c;
    std::vector<Symbol> unk88;
    std::vector<Symbol> unk94;
    std::vector<bool> unka0;
};
