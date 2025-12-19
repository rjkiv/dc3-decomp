#pragma once
#include "HamProfile.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"

class AccomplishmentCountConditional : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentCountConditional();
    virtual bool IsFulfilled(HamProfile *) const;

    AccomplishmentCountConditional(DataArray *, int);

private:
    void Configure(DataArray *);
};
