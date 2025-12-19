#pragma once
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class AccomplishmentCharacterListConditional : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentCharacterListConditional();

    AccomplishmentCharacterListConditional(DataArray *, int);
    bool AreOldOutfitListConditionsMet();
    bool AreCharacterListConditionsMet(Symbol, HamPlayerData *, HamProfile *);
    bool AreUnlockableOutfitListConditionsMet(HamPlayerData *, HamProfile *);

    std::vector<Symbol> unk70;
    std::vector<Symbol> unk7c;
    std::vector<Symbol> unk88;
    std::vector<Symbol> unk94;
    std::vector<bool> unka0;

private:
    void Configure(DataArray *);
};
