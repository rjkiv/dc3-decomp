#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"
#include "utl/Symbol.h"

class AccomplishmentOneShot : public AccomplishmentConditional {
public:
    AccomplishmentOneShot(DataArray *, int);
    virtual ~AccomplishmentOneShot();
    virtual AccomplishmentType GetType() const { return kAccomplishmentTypeOneShot; }

    bool AreOneShotConditionsMet(HamPlayerData *, class HamProfile *, Symbol, Difficulty);

private:
    void Configure(DataArray *);
};
