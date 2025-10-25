#pragma once

#include "obj/Data.h"
#include "utl/Symbol.h"
struct AwardEntry {
public:
    Symbol unk4;
};

class Award {
public:
    virtual ~Award();

    Award(DataArray *, int);
    char const *GetArt() const;
    void GrantAwards(class HamProfile *);
    Symbol GetDisplayName() const;

    Symbol unk4;
    int unk8;
    bool unkc;
    bool unkd;
    std::list<AwardEntry> unk10;
    Symbol unk18;

protected:
    virtual void Configure(DataArray *);

    void GrantAward(AwardEntry const &, HamProfile *);
};
