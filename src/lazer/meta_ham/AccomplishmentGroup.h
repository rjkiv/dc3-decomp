#pragma once

#include "obj/Data.h"
#include "utl/Symbol.h"
class AccomplishmentGroup {
public:
    virtual ~AccomplishmentGroup();

    AccomplishmentGroup(DataArray *, int);
    bool HasAward() const;
    Symbol GetName() const;

    Symbol mName;
    int unk8;
    Symbol mAward;

protected:
    virtual void Configure(DataArray *);
};
