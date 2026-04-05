#pragma once
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/Accomplishment.h"
#include "obj/Data.h"

class AccomplishmentCampaignConditional : public AccomplishmentConditional {
public:
    AccomplishmentCampaignConditional(DataArray *, int);
    virtual ~AccomplishmentCampaignConditional();
    virtual AccomplishmentType GetType() const {
        return kAccomplishmentTypeCampaignConditional;
    }
    virtual bool IsFulfilled(class HamProfile *) const;
    virtual bool IsRelevantForSong(Symbol) const { return true; }

private:
    void Configure(DataArray *);
};
