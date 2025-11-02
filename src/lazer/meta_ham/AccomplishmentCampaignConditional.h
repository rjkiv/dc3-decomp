#pragma once

#include "lazer/meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"

class AccomplishmentCampaignConditional : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentCampaignConditional();
    virtual bool IsFulfilled(class HamProfile *) const;

    AccomplishmentCampaignConditional(DataArray *, int);

private:
    void Configure(DataArray *);
};
