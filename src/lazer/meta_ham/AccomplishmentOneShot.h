#pragma once

#include "hamobj/Difficulty.h"
#include "hamobj/HamPlayerData.h"
#include "lazer/meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"
#include "utl/Symbol.h"
class AccomplishmentOneShot : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentOneShot();

    AccomplishmentOneShot(DataArray *, int);
    bool AreOneShotConditionsMet(HamPlayerData *, class HamProfile *, Symbol, Difficulty);

private:
    void Configure(DataArray *);
};
