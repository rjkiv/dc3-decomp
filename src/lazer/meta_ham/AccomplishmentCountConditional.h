#pragma once
#include "HamProfile.h"
#include "meta_ham/Accomplishment.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "obj/Data.h"

class AccomplishmentCountConditional : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentCountConditional();
    virtual AccomplishmentType GetType() const {
        return kAccomplishmentTypeCountConditional;
    }
    virtual bool IsFulfilled(HamProfile *) const;
    virtual bool IsRelevantForSong(Symbol) const { return true; }

    AccomplishmentCountConditional(DataArray *, int);

private:
    void Configure(DataArray *);
};
